//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
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
		std::map< std::string, types::parse::parameter_set& > parameter_sets;
		for(auto& set: config.sets) parameter_sets.emplace(set.name, set);

		types::merge::config result;

		for(auto& module: config.modules){
			auto pair = result.modules.emplace(
				std::move(module.name),
				types::merge::module{std::move(module.type_name), {}}
			);

			// successfully inserted
			assert(pair.second);

			auto& result_module = pair.first->second;

			// add all parameters from module
			for(auto& parameter: boost::adaptors::reverse(module.parameters)){
				result_module.parameters.emplace(
					std::move(parameter.key),
					std::move(parameter.value)
				);
			}

			// add all parameters from the last to the first parameter set
			// (skip already existing ones)
			for(auto& set: boost::adaptors::reverse(module.parameter_sets)){
				auto iter = parameter_sets.find(set);

				// set was found
				assert(iter != parameter_sets.end());

				for(auto& parameter: iter->second.parameters){
					// parameter set parameters can not be moved
					// (multiple use!)
					result_module.parameters.emplace(
						parameter.key,
						parameter.value
					);
				}
			}
		}

		for(auto& chain: config.chains){
			auto group = chain.group.value_or("default");
			result.chains.emplace_back(types::merge::chain{
				std::move(chain.name),
				std::move(chain.id_generator).value_or(group),
				group, {}
			});

			auto& result_chain = result.chains.back();

			for(auto& module: chain.modules){
				auto iter = result.modules.find(module.name);

				// module was found
				assert(iter != result.modules.end());

				result_chain.modules.emplace_back(types::merge::chain_module{
					*iter, std::move(module.inputs), std::move(module.outputs)
				});
			}
		}

		return result;
	}


}
