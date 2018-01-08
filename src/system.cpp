//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
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
				data.location() + "component type(" + data.type_name
				+ ") is unknown!");
		}

		try{
			return iter->second(data, system);
		}catch(std::exception const& error){
			throw std::runtime_error(data.location() + error.what());
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
				[&config_chain](logsys::stdlogb& os, bool const* active){
					os << "chain(" << config_chain.name << ") analysed";
					if(!active || *active) return;
					os << " and deactivated because it refers to at least one "
						"unknown component module (WARNING)";
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
				// TODO: clang BUG 34749
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
		std::lock_guard lock(change_mutex_);

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

				return logsys::log(
					[](logsys::stdlogb& os){ os << "created embedded config"; },
					[&config]{
						auto result = create_embedded_config(std::move(config));
						set_output_use_count(result);
						return result;
					});
			});

		logsys::log([](logsys::stdlogb& os){ os << "components created"; },
			[this, &config]{
				for(auto& config_component: config.components){
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
								*this)
						);
					});
				}
			});

		logsys::log([](logsys::stdlogb& os){ os << "chains created"; },
			[this, &config]{
				std::tie(inactive_chains_, chains_, id_generators_) =
					disposer::create_chains(
						directory_.module_maker_list_,
						directory_.component_module_maker_list_,
						config.chains);
			});

		load_config_file_valid_ = false;
	}

	void system::remove_component(std::string const& name){
		std::lock_guard lock(change_mutex_);

		load_config_file_valid_ = false;
	}

	void system::load_component(std::istream& content){
		std::lock_guard lock(change_mutex_);

		load_config_file_valid_ = false;
	}

	void system::remove_chain(std::string const& name){
		std::lock_guard lock(change_mutex_);

		load_config_file_valid_ = false;
	}

	void system::load_chain(std::istream& content){
		std::lock_guard lock(change_mutex_);

		load_config_file_valid_ = false;
	}


	enabled_chain system::enable_chain(std::string const& chain){
		auto iter = chains_.find(chain);
		if(iter != chains_.end()) return iter->second;
		if(inactive_chains_.find(chain) != inactive_chains_.end()){
			throw std::runtime_error("chain(" + chain + ") is inactive, "
				"at least one of its modules referes to a unknown component");
		}
		throw std::logic_error("chain(" + chain + ") does not exist");
	}

	std::unordered_set< std::string > system::chains()const{
		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}


}
