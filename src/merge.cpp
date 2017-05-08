//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/merge.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <iterator>
#include <cassert>


namespace disposer{


	types::merge::config merge(types::parse::config&& config){
		using boost::adaptors::reverse;

		std::map< std::string, types::parse::parameter_set& > parameter_sets;
		for(auto& set: config.sets) parameter_sets.emplace(set.name, set);

		types::merge::config result;

		for(auto& chain: config.chains){
			auto group = chain.group.value_or("default");
			result.emplace_back(types::merge::chain{
				std::move(chain.name),
				std::move(chain.id_generator).value_or(group),
				group, {}
			});

			auto& result_chain = result.back();

			for(auto& module: chain.modules){
				std::map< std::string, parameter_data > parameters;

				// add all parameters from module
				for(auto& parameter: reverse(module.parameters.parameters)){
					std::map< std::string, std::string > specialized_values;
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
				for(auto& set: reverse(module.parameters.parameter_sets)){
					auto iter = parameter_sets.find(set);

					// set was found
					assert(iter != parameter_sets.end());

					for(auto& parameter: iter->second.parameters){
						// parameter set parameters can not be moved
						// (multiple use!)
						std::map< std::string, std::string > specialized_values;
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

				result_chain.modules.emplace_back(types::merge::module{
					std::move(module.type_name),
					std::move(parameters),
					std::move(module.inputs),
					std::move(module.outputs)
				});
			}
		}

		return result;
	}


}
