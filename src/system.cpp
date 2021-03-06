//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/system.hpp>
#include <disposer/core/module_base.hpp>
#include <disposer/core/component_base.hpp>

#include <disposer/config/component_make_data.hpp>
#include <disposer/config/check_semantic.hpp>
#include <disposer/config/unused_parameter_set_warning.hpp>
#include <disposer/config/set_output_use_count.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <fstream>


namespace disposer{ namespace{


	component_ptr create_component(
		component_maker_list const& module_makers,
		component_make_data const& data,
		system& system
	){
		auto iter = module_makers.find(data.type_name);

		if(iter == module_makers.end()){
			throw std::logic_error(
				data.log_prefix() + "component type(" + data.type_name
				+ ") is unknown!");
		}

		try{
			return iter->second(data, system);
		}catch(std::exception const& error){
			throw std::runtime_error(data.log_prefix() + error.what());
		}
	}


	auto create_chains(
		module_maker_list const& module_makers,
		component_module_makers_list& component_module_makers,
		types::embedded_config::chains_config const& config
	){
		std::unordered_set< std::string > inactive_chains;
		std::unordered_map< std::string, chain > chains;
		std::unordered_map< std::string, id_generator > id_generators;

		for(auto& config_chain: config){
			bool const chain_is_active = logsys::log(
				[&config_chain](
					logsys::stdlogb& os,
					std::optional< bool > active
				){
					os << "chain(" << config_chain.name << ") analysed";
					if(!active || *active) return;
					os << " and deactivated because it refers to at least "
						"one unknown component module (WARNING)";
				}, [&config_chain, &component_module_makers]{
					for(auto const& module: config_chain.modules){
						auto const& name = module.type_name;
						auto const pos = name.find("//");
						if(pos == std::string::npos) continue;
						auto const component_name = name.substr(0, pos);
						if(component_module_makers.find(component_name)
							!= component_module_makers.end()) continue;
						return false;
					}

					return true;
				});

			if(!chain_is_active){
				inactive_chains.emplace(config_chain.name);
				continue;
			}

			logsys::log([&config_chain](logsys::stdlogb& os){
					os << "chain(" << config_chain.name << ") created";
				}, [&]{
					// emplace the new process chain
					chains.try_emplace(
							config_chain.name,
							module_makers,
							component_module_makers,
							config_chain,
							id_generators[config_chain.id_generator]
						);
				});
		}

		return std::tuple(
			std::move(inactive_chains),
			std::move(chains),
			std::move(id_generators)
		);
	}


}}


namespace disposer{


	system::~system(){
		for(auto& [name, component]: components_){
			logsys::exception_catching_log(
				[&name = name](logsys::stdlogb& os){
					os << "component(" << name << ") shutdown";
				},
				[&component = component]{ component->shutdown(); });
		}
	}


	void system::load_config_file(std::string const& filename){
		std::ifstream is = logsys::log(
			[&filename](logsys::stdlogb& os){
				os << "open '" << filename << "'";
			},
			[&filename]{
				std::ifstream is(filename.c_str());
				if(!is.is_open()){
					throw std::runtime_error("Can not open '" + filename + "'");
				}
				return is;
			});

		load_config(is);
	}

	void system::load_config(std::istream& content){
		std::lock_guard lock(mutex_);

		auto config = logsys::log(
			[](logsys::stdlogb& os){ os << "config file loaded"; },
			[this, &content]{
				if(!load_config_file_valid_){
					throw std::logic_error(
						"system::load_config is called multiple times");
				}

				auto config = logsys::log([&](logsys::stdlogb& os){
						os << "parsed content";
					}, [&]{ return parse(content); });

				logsys::log(
					[](logsys::stdlogb& os){ os << "checked semantic"; },
					[&config]{ check_semantic(config); });

				logsys::log([](logsys::stdlogb& os){
						os << "looked for unused stuff and warned about it";
					}, [&config]{ unused_parameter_set_warning(config); });

				return config;
			});

		auto const embedded_config = logsys::log(
			[](logsys::stdlogb& os){ os << "created embedded config"; },
			[&config]{
				auto result = create_embedded_config(config);
				set_output_use_count(result);
				return result;
			});

		logsys::log([](logsys::stdlogb& os){ os << "components created"; },
			[this, &embedded_config]{
				for(auto& config_component: embedded_config.components){
					logsys::log([&config_component](logsys::stdlogb& os){
							os << "component(" << config_component.name << ":"
								<< config_component.type_name << ") created";
						}, [&]{
							// emplace the new process chain
							components_.emplace(
								config_component.name,
								create_component(
									directory_.component_maker_list_,
									{
										config_component.name,
										config_component.type_name,
										config_component.parameters
									},
									*this));
						});
				}
			});

		logsys::log([](logsys::stdlogb& os){ os << "chains created"; },
			[this, &embedded_config, &config]{
				std::tie(inactive_chains_, chains_, id_generators_) =
					disposer::create_chains(
						directory_.module_maker_list_,
						directory_.component_module_maker_list_,
						embedded_config.chains);
				config_ = std::move(config);
			});

		load_config_file_valid_ = false;
	}

	void system::remove_component(std::string const& name){
		std::lock_guard lock(mutex_);

		auto const iter = components_.find(name);
		if(iter == components_.end()){
			throw std::logic_error("component(" + name + ") doesn't exist");
		}

		auto maker_iter = directory_.component_module_maker_list_.find(name);
		if(maker_iter != directory_.component_module_maker_list_.end()){
			auto& [component_name, component] = *maker_iter;
			for(auto& [module_name, module]: component){
				if(module.usage_count > 0){
					throw std::logic_error("can't remove component("
						+ component_name + ") because its module("
						+ module_name + ") is still in use");
				}
			}
		}

		directory_.component_module_maker_list_.erase(maker_iter);
		components_.erase(iter);

		load_config_file_valid_ = false;
	}

	void system::load_component(std::istream& content){
		std::lock_guard lock(mutex_);

		auto config = parse_component(content);

		if(components_.find(config.name) != components_.end()){
			throw std::logic_error("a component(" + config.name
				+ ") already exists");
		}

		check_semantic(config_.sets, config);

		auto const embedded_config =
			create_embedded_config(config_.sets, config);

		logsys::log([&embedded_config](logsys::stdlogb& os){
				os << "component(" << embedded_config.name << ") created";
			}, [&]{
				components_.emplace(
					embedded_config.name,
					create_component(
						directory_.component_maker_list_,
						{
							embedded_config.name,
							embedded_config.type_name,
							embedded_config.parameters
						},
						*this));

				config_.components.push_back(std::move(config));
			});

		load_config_file_valid_ = false;
	}

	void system::remove_chain(std::string const& name){
		std::lock_guard lock(mutex_);

		auto const iter = chains_.find(name);
		if(iter == chains_.end()){
			throw std::logic_error("chain(" + name + ") doesn't exist");
		}

		if(iter->second.is_enabled()){
			throw std::logic_error("chain(" + name +
				") is currently in use and therefore cannot be deleted");
		}

		chains_.erase(iter);

		load_config_file_valid_ = false;
	}

	void system::load_chain(std::istream& content){
		std::lock_guard lock(mutex_);

		auto config = parse_chain(content);

		if(chains_.find(config.name) != chains_.end()){
			throw std::logic_error("a chain(" + config.name
				+ ") already exists");
		}

		if(inactive_chains_.find(config.name) != inactive_chains_.end()){
			throw std::logic_error("a chain(" + config.name
				+ ") already exists even though it is inactive");
		}

		check_semantic(config_.sets, config);

		auto embedded_config = create_embedded_config(config_.sets, config);

		set_output_use_count(embedded_config);

		logsys::log([&embedded_config](logsys::stdlogb& os){
				os << "chain(" << embedded_config.name << ") created";
			}, [&]{
				// emplace the new process chain
				chains_.try_emplace(
						embedded_config.name,
						directory_.module_maker_list_,
						directory_.component_module_maker_list_,
						embedded_config,
						id_generators_[embedded_config.id_generator]
					);

				config_.chains.push_back(std::move(config));
			});

		load_config_file_valid_ = false;
	}


	chain& system::get_chain(std::string const& chain){
		std::lock_guard lock(mutex_);

		auto iter = chains_.find(chain);
		if(iter != chains_.end()) return iter->second;
		if(inactive_chains_.find(chain) != inactive_chains_.end()){
			throw std::runtime_error("chain(" + chain + ") is inactive, "
				"at least one of its modules referes to a unknown component");
		}
		throw std::logic_error("chain(" + chain + ") does not exist");
	}

	std::unordered_set< std::string > system::chains()const{
		std::lock_guard lock(mutex_);

		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}


}
