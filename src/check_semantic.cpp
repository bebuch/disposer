//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/config/check_semantic.hpp>

#include <unordered_map>
#include <unordered_set>
#include <sstream>


namespace disposer{ namespace{


	template < typename LocationFn >
	void check_params(
		LocationFn const& location_fn,
		std::vector< types::parse::parameter > const& params
	){
		std::unordered_set< std::string > keys;
		for(auto& param: params){
			if(!keys.insert(param.key).second){
				throw std::logic_error(
					location_fn() + "duplicate key '" + param.key + "'"
				);
			}

			std::unordered_set< std::string > types;
			for(auto& specialization: param.specialized_values){
				if(!keys.insert(specialization.type).second){
					throw std::logic_error(
						location_fn() + "duplicate parameter "
						"specialization type '" + specialization.type
						+ "' for parameter '" + param.key + "'"
					);
				}
			}
		}
	}

	template < typename LocationFn >
	void check_param_sets(
		LocationFn const& location_fn,
		std::unordered_set< std::string > const& known_sets,
		std::vector< std::string > const& param_sets
	){
		std::unordered_set< std::string > sets;
		for(auto& set: param_sets){
			if(known_sets.find(set) == known_sets.end()){
				throw std::logic_error(
					location_fn() + "unknown parameter_set '" + set + "'"
				);
			}

			if(!sets.insert(set).second){
				throw std::logic_error(
					location_fn() + "duplicate use of parameter_set '"
					+ set + "'"
				);
			}
		}
	}


	struct connected{
		connected(
			std::string const& chain_name,
			std::string const& module_type_name,
			std::size_t const& module_number,
			std::string const& output_name
		)
			: chain_name(chain_name)
			, module_type_name(module_type_name)
			, module_number(module_number)
			, output_name(output_name) {}

		std::string const chain_name;
		std::string const module_type_name;
		std::size_t const module_number;
		std::string const output_name;
	};


	std::unordered_set< std::string > check_semantic(
		types::parse::parameter_sets const& config
	){
		std::unordered_set< std::string > known_sets;
		for(auto& set: config){
			if(!known_sets.insert(set.name).second){
				throw std::logic_error(
					"in parameter_set list: duplicate name '" + set.name + "'"
				);
			}

			std::unordered_set< std::string > keys;
			for(auto& param: set.parameters){
				if(!keys.insert(param.key).second){
					throw std::logic_error(
						"in parameter_set '" + set.name +
						"': duplicate key '" + param.key + "'"
					);
				}
			}
		}

		return known_sets;
	}

	void check_semantic(
		types::parse::components const& config,
		std::unordered_set< std::string > const& known_sets
	){
		std::unordered_set< std::string > components;
		for(auto& component: config){
			if(!components.insert(component.name).second){
				throw std::logic_error(
					"in component list: duplicate name '" + component.name + "'"
				);
			}

			auto location = [&component]{
				return "in component(" + component.name + ") of type("
					+ component.type_name + "): ";
			};

			check_param_sets(location, known_sets,
				component.parameters.parameter_sets);
			check_params(location, component.parameters.parameters);
		}
	}

	void check_semantic(
		types::parse::chain const& chain,
		std::unordered_set< std::string > const& known_sets
	){
		std::unordered_map< std::string, connected > variables;
		std::size_t module_number = 1;
		for(auto& module: chain.modules){
			auto location = [&chain, &module, module_number]{
				return "in chain(" + chain.name + ") module("
					+ std::to_string(module_number) + ":"
					+ module.type_name + "): ";
			};

			check_param_sets(location, known_sets,
				module.parameters.parameter_sets);
			check_params(location, module.parameters.parameters);

			std::unordered_set< std::string > inputs;
			for(auto& input: module.inputs){
				if(!inputs.insert(input.name).second){
					throw std::logic_error(
						location() + "duplicate input '" + input.name + "'"
					);
				}
			}

			for(auto& input: module.inputs){
				auto const iter = variables.find(input.variable);
				if(iter == variables.end()){
					throw std::logic_error(
						location() + "unknown variable '" +
						input.variable + "' as input of '" + input.name +
						"'"
					);
				}

				if(input.transfer == in_transfer::move){
					variables.erase(iter);
				}
			}

			std::unordered_set< std::string > outputs;
			for(auto& output: module.outputs){
				if(!outputs.insert(output.name).second){
					throw std::logic_error(
						location() + "duplicate output '" +
						output.name + "'"
					);
				}

				if(!variables.try_emplace(
						output.variable,
						chain.name,
						module.type_name,
						module_number,
						output.name
					).second
				){
					throw std::logic_error(
						location() + "duplicate use of variable '" +
						output.variable + "' as output of '" +
						output.name + "'"
					);
				}
			}

			++module_number;
		}

		if(!variables.empty()){
			std::ostringstream os;
			os << "Some variables are never finally used: ";
			bool first = true;
			for(auto const& [name, data]: variables){
				if(first){ first = false; }else{ os << ", "; }
				os << "variable(" << name << ") from chain("
					<< data.chain_name << ") module("
					<< data.module_number << ":"
					<< data.module_type_name << ") output("
					<< data.output_name << ")";
			}

			throw std::logic_error(os.str());
		}
	}

	void check_semantic(
		types::parse::chains const& config,
		std::unordered_set< std::string > const& known_sets
	){
		std::unordered_set< std::string > chains;
		for(auto& chain: config){
			if(!chains.insert(chain.name).second){
				throw std::logic_error(
					"in chain list: duplicate name '" + chain.name + "'"
				);
			}

			check_semantic(chain, known_sets);
		}
	}


}}


namespace disposer{


	void check_semantic(types::parse::component const& component){
		auto location = [&component]{
			return "in component(" + component.name + ") of type("
				+ component.type_name + "): ";
		};

		if(component.parameters.parameter_sets.size() > 0){
			std::logic_error(location() + "when creating a single component, "
				"the parsed config must not refer to any parameter_set's");
		}

		check_params(location, component.parameters.parameters);
	}

	void check_semantic(types::parse::chain const& chain){
		static std::unordered_set< std::string > const known_sets;
		check_semantic(chain, known_sets);
	}

	void check_semantic(types::parse::config const& config){
		auto const known_sets = check_semantic(config.sets);
		check_semantic(config.components, known_sets);
		check_semantic(config.chains, known_sets);
	}


}
