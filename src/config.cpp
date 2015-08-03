//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/config.hpp>

#include <disposer/module_base.hpp>
#include <disposer/check_semantic.hpp>
#include <disposer/unused_warnings.hpp>
#include <disposer/merge.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


namespace disposer{ namespace config{


	chain_list load(std::string const& filename){
		auto config = parse("plan.ini");

		disposer::check_semantic(config);
		disposer::unused_warnings(config);

		auto merged_config = disposer::merge(std::move(config));

		chain_list result;
		for(auto& chain: merged_config.chains){
			std::vector< module_ptr > modules;

			std::map< std::string, std::pair< impl::output::output_entry&, bool > > variables;

			bool first = true;
			for(auto& module: chain.modules){
				io_list inputs;
				for(auto& input: module.inputs){
					inputs.insert(input.name);
				}

				io_list outputs;
				for(auto& output: module.outputs){
					outputs.insert(output.name);
				}

				modules.push_back(make_module(
					module.module.second.type,
					chain.name,
					module.module.first,
					inputs,
					outputs,
					parameter_processor(module.module.second.parameters),
					first
				));

				for(auto& output: module.outputs){
					auto entry = modules.back()->outputs.find(output.name);
					assert(entry != modules.back()->outputs.end());

					variables.emplace(output.variable, std::pair< impl::output::output_entry&, bool >(entry->second, true));
				}

				first = false;
			}

			auto module_ptr_iter = modules.end();
			for(auto& module: boost::adaptors::reverse(chain.modules)){
				--module_ptr_iter;
				auto& module_ptr = *module_ptr_iter;

				for(auto& input_name_and_var: module.inputs){
					auto output_iter = variables.find(input_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;
					auto& last_use = output_iter->second.second;

					auto input_iter = module_ptr->inputs.find(input_name_and_var.name);
					assert(input_iter != module_ptr->inputs.end());

					auto& input = input_iter->second;

					if(output.type != input.object.type){
						throw std::runtime_error(
							"In chain '" + chain.name +
							"' module '" + module_ptr->name +
							"': Variable '" + input_name_and_var.variable +
							"' is incompatible with input '" + input_name_and_var.name +
							"' (input type: '" + input.object.type.pretty_name() +
							"'; variable type: '" + output.type.pretty_name() +
							"')");
					}

					output.connect(input, last_use);
					last_use = false;
				}
			}

			result.emplace(chain.name, disposer::chain(std::move(modules), chain.increase));
		}

		return result;
	}


} }
