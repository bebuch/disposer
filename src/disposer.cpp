//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/module_base.hpp>
#include <disposer/core/component_base.hpp>

#include <disposer/config/component_make_data.hpp>
#include <disposer/config/check_semantic.hpp>
#include <disposer/config/unused_warnings.hpp>
#include <disposer/config/set_output_use_count.hpp>

#include <disposer/disposer.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>


namespace disposer{ namespace{


	component_ptr create_component(
		component_maker_list const& maker_list,
		component_make_data const& data
	){
		auto iter = maker_list.find(data.type_name);

		if(iter == maker_list.end()){
			throw std::logic_error(
				data.location() + "component type(" + data.type_name
				+ ") is unknown!");
		}

		try{
			return iter->second(data);
		}catch(std::exception const& error){
			throw std::runtime_error(data.location() + error.what());
		}
	}


	auto create_chains(
		module_maker_list const& maker_list,
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
				}, [&config_chain, &maker_list]{
					for(auto const& module: config_chain.modules){
						auto const& name = module.type_name;
						if(name.find("//") == std::string::npos) continue;
						if(maker_list.find(name) != maker_list.end()) continue;
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
					maker_list,
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


	disposer::disposer()
		: component_declarant_(*this)
		, module_declarant_(*this) {}

	disposer::~disposer(){
		for(auto& [name, component]: components_){
			(void)name;
			component->shutdown();
		}
	}

	void component_declarant::operator()(
		std::string const& type_name,
		component_maker_fn&& fn
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add component type name '" << type_name << "'";
		}, [&]{
			auto iter = disposer_.component_maker_list_.insert(
				std::make_pair(type_name, std::move(fn)));

			if(!iter.second){
				throw std::logic_error(
					"component type name '" + type_name
					+ "' has been added more than one time!"
				);
			}
		});
	}

	void component_declarant::help(
		std::string const& type_name,
		std::string&& text
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add help for component type name '" << type_name << "'";
		}, [&]{
			auto iter = disposer_.component_help_list_.insert(
				std::make_pair(type_name, std::move(text)));

			if(!iter.second){
				throw std::logic_error(
					"component type name '" + type_name
					+ "' help text has been added more than one time!"
				);
			}
		});
	}

	void module_declarant::operator()(
		std::string const& type_name,
		module_maker_fn&& fn
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add module type name '" << type_name << "'";
		}, [&]{
			auto iter = disposer_.module_maker_list_.insert(
				std::make_pair(type_name, std::move(fn)));

			if(!iter.second){
				throw std::logic_error(
					"module type name '" + type_name
					+ "' has been added more than one time!"
				);
			}
		});
	}

	void module_declarant::help(
		std::string const& type_name,
		std::string&& text
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add help for module type name '" << type_name << "'";
		}, [&]{
			auto iter = disposer_.module_help_list_.insert(
				std::make_pair(type_name, std::move(text)));

			if(!iter.second){
				throw std::logic_error(
					"module type name '" + type_name
					+ "' help text has been added more than one time!"
				);
			}
		});
	}


	component_declarant& disposer::component_declarant(){
		return component_declarant_;
	}

	module_declarant& disposer::module_declarant(){
		return module_declarant_;
	}

	void disposer::load(std::string const& filename){
		auto config = logsys::log([&](logsys::stdlogb& os){
				os << "parsed '" << filename << "'";
			}, [&]{ return parse(filename); });

		logsys::log([](logsys::stdlogb& os){ os << "checked semantic"; },
			[&config]{ check_semantic(config); });

		logsys::log([](logsys::stdlogb& os){
				os << "looked for unused stuff and warned about it";
			}, [&config]{ unused_warnings(config); });

		config_ = logsys::log(
			[](logsys::stdlogb& os){ os << "created embedded config"; },
			[&config]{
				auto result = create_embedded_config(std::move(config));
				set_output_use_count(result);
				return result;
			});
	}

	void disposer::create_components(){
		for(auto& config_component: config_.components){
			logsys::log([&config_component](logsys::stdlogb& os){
				os << "component(" << config_component.name << ":"
					<< config_component.type_name << ") created";
			}, [&]{
				// emplace the new process chain
				components_.emplace(
					config_component.name,
					create_component(component_maker_list_, {
							config_component.name,
							config_component.type_name,
							config_component.parameters
						})
				);
			});
		}
	}

	void disposer::create_chains(){
		std::tie(inactive_chains_, chains_, id_generators_) =
			::disposer::create_chains(module_maker_list_, config_.chains);
	}


	std::string disposer::help()const{
		std::string result;
		for(auto const& component: component_help_list_){
			result += component.second;
		}
		for(auto const& module: module_help_list_){
			result += module.second;
		}
		return result;
	}

	std::string disposer::module_help(std::string const& name)const{
		auto const iter = module_help_list_.find(name);
		if(iter == module_help_list_.end()){
			throw std::logic_error(
				std::string("no module '") + name + "' found");
		}
		return iter->second;
	}

	std::string disposer::component_help(std::string const& name)const{
		auto const iter = component_help_list_.find(name);
		if(iter == component_help_list_.end()){
			throw std::logic_error(
				std::string("no component '") + name + "' found");
		}
		return iter->second;
	}

	std::vector< std::string > disposer::module_names()const{
		std::vector< std::string > modules;
		modules.reserve(module_help_list_.size());
		for(auto const& module: module_help_list_){
			modules.push_back(module.first);
		}
		return modules;
	}

	std::vector< std::string > disposer::component_names()const{
		std::vector< std::string > components;
		components.reserve(component_help_list_.size());
		for(auto const& component: component_help_list_){
			components.push_back(component.first);
		}
		return components;
	}



	chain& disposer::get_chain(std::string const& chain){
		auto iter = chains_.find(chain);
		if(iter != chains_.end()) return iter->second;
		if(inactive_chains_.find(chain) != inactive_chains_.end()){
			throw std::runtime_error("chain(" + chain + ") is inactive, "
				"at least one of its modules referes to a unknown component");
		}
		throw std::logic_error("chain(" + chain + ") does not exist");
	}

	std::unordered_set< std::string > disposer::chains()const{
		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}


}
