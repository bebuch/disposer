//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/disposer_directory.hpp>


namespace disposer{


	std::string disposer_directory::help()const{
		std::string result;
		for(auto const& component: component_help_list_){
			result += component.second;
		}
		for(auto const& module: module_help_list_){
			result += module.second;
		}
		return result;
	}

	std::string disposer_directory::module_help(std::string const& name)const{
		auto const iter = module_help_list_.find(name);
		if(iter == module_help_list_.end()){
			throw std::logic_error(
				std::string("no module '") + name + "' found");
		}
		return iter->second;
	}

	std::string disposer_directory::component_help(std::string const& name)const{
		auto const iter = component_help_list_.find(name);
		if(iter == component_help_list_.end()){
			throw std::logic_error(
				std::string("no component '") + name + "' found");
		}
		return iter->second;
	}

	std::vector< std::string > disposer_directory::module_names()const{
		std::vector< std::string > modules;
		modules.reserve(module_help_list_.size());
		for(auto const& module: module_help_list_){
			modules.push_back(module.first);
		}
		return modules;
	}

	std::vector< std::string > disposer_directory::component_names()const{
		std::vector< std::string > components;
		components.reserve(component_help_list_.size());
		for(auto const& component: component_help_list_){
			components.push_back(component.first);
		}
		return components;
	}


}
