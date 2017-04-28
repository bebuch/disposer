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
				std::map< std::string, std::string > parameters;

				// add all parameters from module
				for(auto& parameter: reverse(module.parameters.parameters)){
					parameters.emplace(
						std::move(parameter.key),
						std::move(parameter.value)
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
						parameters.emplace(
							parameter.key,
							parameter.value
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
