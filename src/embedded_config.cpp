//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/config/embedded_config.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <iterator>
#include <cassert>


namespace disposer{ namespace{


	using param_sets_map = std::map<
		std::string, types::parse::parameter_set const&, std::less<> >;

	auto map_name_to_set(types::parse::parameter_sets const& sets){
		param_sets_map parameter_sets;
		for(auto const& set: sets) parameter_sets.emplace(set.name, set);
		return parameter_sets;
	}


	auto embedded_config_parameters(
		param_sets_map const& sets,
		types::parse::parameters&& params
	){
		using boost::adaptors::reverse;

		parameter_list parameters;

		// add all parameters from module
		for(auto& parameter: reverse(params.parameters)){
			std::map< std::string, std::string, std::less<> >
				specialized_values;
			for(auto& specialization: parameter.specialized_values){
				specialized_values.emplace(
					std::move(specialization.type),
					std::move(specialization.value)
				);
			}

			parameters.emplace(
				std::move(parameter.key), parameter_data{
					std::move(parameter.generic_value),
					std::move(specialized_values)
				}
			);
		}

		// add all parameters from the last to the first parameter set
		// (skip already existing ones)
		for(auto& set: reverse(params.parameter_sets)){
			auto iter = sets.find(set);

			// set was found
			assert(iter != sets.end());

			for(auto& parameter: iter->second.parameters){
				// parameter set parameters can not be moved
				// (multiple use!)
				std::map< std::string, std::string, std::less<> >
					specialized_values;
				for(auto& specialization: parameter.specialized_values){
					specialized_values.emplace(
						specialization.type,
						specialization.value
					);
				}

				parameters.emplace(
					parameter.key, parameter_data{
						parameter.generic_value,
						std::move(specialized_values)
					}
				);
			}
		}

		return parameters;
	}


	auto embedded_config_components(
		param_sets_map const& sets,
		types::parse::components&& components
	){
		types::embedded_config::components_config result;
		for(auto& component: components){
			result.push_back({
				std::move(component.name),
				std::move(component.type_name),
				embedded_config_parameters(sets,
					std::move(component.parameters))
			});
		}
		return result;
	}

	auto embedded_config_chains(
		param_sets_map const& sets,
		types::parse::chains&& chains
	){
		types::embedded_config::chains_config result;
		for(auto& chain: chains){
			auto& result_chain = result.emplace_back(
				types::embedded_config::chain{
					std::move(chain.name),
					chain.id_generator.value_or("default"),
					{}
				});

			std::vector< std::string > module_types;
			for(auto& module: chain.modules){
				std::vector< std::size_t > wait_ons;
				wait_ons.reserve(module.wait_ons.size());
				for(auto& wait_on: module.wait_ons){
					auto const module_number = module_types.size();

					auto location = [&result_chain, &module, module_number]{
						return "in chain(" + result_chain.name + ") module("
							+ std::to_string(module_number) + ":"
							+ module.type_name + "): ";
					};

					if(wait_on.number == 0){
						throw std::logic_error(location() +
							"wait_on number must not be 0");
					}

					if(wait_on.number > module_number){
						throw std::logic_error(location() + "wait_on number "
							+ std::to_string(wait_on.number)
							+ " is greater than current module number");
					}

					if(wait_on.type_name != module_types[wait_on.number - 1]){
						throw std::logic_error(location()
							+ "wait_on referes to module("
							+ std::to_string(wait_on.number) + ":"
							+ wait_on.type_name +
							+ ") but module " + std::to_string(wait_on.number)
							+ "is of type " + module_types[wait_on.number - 1]);
					}

					wait_ons.push_back(wait_on.number - 1);
				}

				std::vector< types::embedded_config::out > outputs;
				outputs.reserve(module.outputs.size());
				for(auto& output: module.outputs){
					outputs.push_back({
						std::move(output.name),
						std::move(output.variable),
						0
					});
				}

				result_chain.modules.push_back({
					module.type_name,
					std::move(wait_ons),
					embedded_config_parameters(sets,
						std::move(module.parameters)),
					std::move(module.inputs),
					std::move(outputs)
				});

				module_types.push_back(std::move(module.type_name));
			}
		}
		return result;
	}


}}


namespace disposer{


	types::embedded_config::config create_embedded_config(
		types::parse::config&& config
	){
		auto sets = map_name_to_set(config.sets);
		return {
			embedded_config_components(sets, std::move(config.components)),
			embedded_config_chains(sets, std::move(config.chains))
		};
	}


}
