//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/config/set_output_use_count.hpp>

#include <unordered_map>
#include <cassert>


namespace disposer{


	void set_output_use_count(types::embedded_config::chain& chain){
		std::unordered_map< std::string, types::embedded_config::out* >
			variables;

		for(auto& module: chain.modules){
			for(auto& input: module.inputs){
				auto const iter = variables.find(input.variable);
				assert(iter != variables.end());

				++iter->second->use_count;

				if(input.transfer == in_transfer::move){
					variables.erase(iter);
				}
			}

			for(auto& output: module.outputs){
				variables.emplace(output.variable, &output);
			}
		}

		assert(variables.empty());
	}

	void set_output_use_count(types::embedded_config::config& config){
		for(auto& chain: config.chains){
			set_output_use_count(chain);
		}
	}


}
