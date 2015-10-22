//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/check_semantic.hpp>

#include <set>


namespace disposer{


	void check_semantic(types::parse::config const& config){
		std::set< std::string > parameter_sets;
		for(auto& set: config.sets){
			if(!parameter_sets.insert(set.name).second){
				throw std::logic_error(
					"In parameter_set list: Duplicate name '" + set.name + "'"
				);
			}

			std::set< std::string > keys;
			for(auto& param: set.parameters){
				if(!keys.insert(param.key).second){
					throw std::logic_error(
						"In parameter_set '" + set.name +
						"': Duplicate key '" + param.key + "'"
					);
				}
			}
		}

		std::set< std::string > modules;
		for(auto& module: config.modules){
			if(!modules.insert(module.name).second){
				throw std::logic_error(
					"In module list: Duplicate name '" + module.name + "'"
				);
			}

			std::set< std::string > sets;
			for(auto& set: module.parameter_sets){
				if(parameter_sets.find(set) == parameter_sets.end()){
					throw std::logic_error(
						"In module '" + module.name +
						"': Unknown parameter_set '" + set + "'"
					);
				}

				if(!sets.insert(set).second){
					throw std::logic_error(
						"In module '" + module.name +
						"': Duplicate use of parameter_set '" + set + "'"
					);
				}
			}

			std::set< std::string > keys;
			for(auto& param: module.parameters){
				if(!keys.insert(param.key).second){
					throw std::logic_error(
						"In module '" + module.name + "': Duplicate key '" +
						param.key + "'"
					);
				}
			}
		}

		std::set< std::string > chains;
		for(auto& chain: config.chains){
			if(!chains.insert(chain.name).second){
				throw std::logic_error(
					"In chain list: Duplicate name '" + chain.name + "'"
				);
			}

			std::set< std::string > variables;
			std::set< std::string > chain_modules;
			for(auto& module: chain.modules){
				if(modules.find(module.name) == modules.end()){
					throw std::logic_error(
						"In chain '" + chain.name + "': Unknown module '" +
						module.name + "'"
					);
				}

				if(!chain_modules.insert(module.name).second){
					throw std::logic_error(
						"In chain '" + chain.name +
						"': Duplicate use of module '" + module.name + "'"
					);
				}

				std::set< std::string > inputs;
				for(auto& input: module.inputs){
					if(variables.find(input.variable) == variables.end()){
						throw std::logic_error(
							"In chain '" + chain.name + "' module '" +
							module.name + "': Unknown variable '" +
							input.variable + "' as input of '" + input.name +
							"'"
						);
					}
				}

				std::set< std::string > outputs;
				for(auto& output: module.outputs){
					if(!outputs.insert(output.name).second){
						throw std::logic_error(
							"In chain '" + chain.name + "' module '" +
							module.name + "': Duplicate output '" +
							output.name + "'"
						);
					}

					if(!variables.insert(output.variable).second){
						throw std::logic_error(
							"In chain '" + chain.name + "' module '" +
							module.name + "': Duplicate use of variable '" +
							output.variable + "' as output of '" +
							output.name + "'"
						);
					}
				}
			}
		}
	}


}
