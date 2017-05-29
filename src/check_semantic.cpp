//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
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
					"in parameter_set list: duplicate name '" + set.name + "'"
				);
			}

			std::set< std::string > keys;
			for(auto& param: set.parameters){
				if(!keys.insert(param.key).second){
					throw std::logic_error(
						"in parameter_set '" + set.name +
						"': duplicate key '" + param.key + "'"
					);
				}
			}
		}

		std::set< std::string > chains;
		for(auto& chain: config.chains){
			if(!chains.insert(chain.name).second){
				throw std::logic_error(
					"in chain list: duplicate name '" + chain.name + "'"
				);
			}

			std::set< std::string > variables;
			std::set< std::string > chain_modules;
			std::size_t module_number = 1;
			for(auto& module: chain.modules){
				auto location = [&chain, &module, module_number]{
					return "in chain '" + chain.name + "' module "
						+ std::to_string(module_number) + " (Type '"
						+ module.type_name + "': ";
				};

				std::set< std::string > sets;
				for(auto& set: module.parameters.parameter_sets){
					if(parameter_sets.find(set) == parameter_sets.end()){
						throw std::logic_error(
							location() + "unknown parameter_set '" + set + "'"
						);
					}

					if(!sets.insert(set).second){
						throw std::logic_error(
							location() + "duplicate use of parameter_set '"
							+ set + "'"
						);
					}
				}

				std::set< std::string > keys;
				for(auto& param: module.parameters.parameters){
					if(!keys.insert(param.key).second){
						throw std::logic_error(
							location() + "duplicate key '" + param.key + "'"
						);
					}

					std::set< std::string > types;
					for(auto& specialization: param.specialized_values){
						if(!keys.insert(specialization.type).second){
							throw std::logic_error(
								location() + "duplicate parameter "
								"specialization type '" + specialization.type
								+ "' for parameter '" + param.key + "'"
							);
						}
					}
				}

				std::set< std::string > inputs;
				for(auto& input: module.inputs){
					if(!inputs.insert(input.name).second){
						throw std::logic_error(
							location() + "duplicate input '" + input.name + "'"
						);
					}
				}

				for(auto& input: module.inputs){
					if(variables.find(input.variable) == variables.end()){
						throw std::logic_error(
							location() + "unknown variable '" +
							input.variable + "' as input of '" + input.name +
							"'"
						);
					}

					if(input.transfer == in_transfer::move){
						variables.erase(input.variable);
					}
				}

				std::set< std::string > outputs;
				for(auto& output: module.outputs){
					if(!outputs.insert(output.name).second){
						throw std::logic_error(
							location() + "duplicate output '" + output.name
							+ "'"
						);
					}
				}

				for(auto& output: module.outputs){
					if(!outputs.insert(output.name).second){
						throw std::logic_error(
							location() + "duplicate output '" +
							output.name + "'"
						);
					}

					if(!variables.insert(output.variable).second){
						throw std::logic_error(
							location() + "duplicate use of variable '" +
							output.variable + "' as output of '" +
							output.name + "'"
						);
					}
				}

				++module_number;
			}
		}
	}


}
