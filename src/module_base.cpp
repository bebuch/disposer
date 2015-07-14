//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "module_base.hpp"
#include "log_tag.hpp"
#include "log.hpp"

#include <unordered_map>


namespace disposer{


	module_base::module_base(std::string const& type, std::string const& chain, std::string const& name):
		type(type),
		chain(chain),
		name(name)
		{}

	auto& maker_list(){
		static std::unordered_map< std::string, maker_function > list;
		return list;
	}

	void add_module_maker(std::string const& type, maker_function&& function){
		auto iter = maker_list().insert(std::make_pair(type, std::move(function)));
		if(!iter.second) throw std::logic_error("Module type '" + type + "' is double registered!");
	}

	module_ptr make_module(std::string const& type, std::string const& chain, std::string const& name, io_list const& inputs, io_list const& outputs, parameter_processor&& parameters, bool is_start){
		auto iter = maker_list().find(type);
		if(iter == maker_list().end()) throw std::runtime_error("Module '" + chain + "." + name + "': " + "Type '" + type + "' is unknown!");
		try{
			auto result = iter->second(type, chain, name, inputs, outputs, parameters, is_start);
			for(auto const& param: parameters.unused_parameters()){
				log([&param, &chain, &name](log_tag& os){ os << "In chain '" << chain << "' module '" << name << ": Unused parameter '" << param.first << "'='" << param.second << "'"; });
			}
			return result;
		}catch(std::exception const& error){
			throw std::runtime_error("Module '" + chain + "." + name + "': " + error.what());
		}
	}


}
