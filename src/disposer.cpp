//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/disposer.hpp>
#include <disposer/check_semantic.hpp>
#include <disposer/unused_warnings.hpp>
#include <disposer/merge.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


namespace disposer{


	void disposer::add_module_maker(std::string const& type, maker_function&& function){
		log([&type](log_base& os){ os << "register module type '" << type << "'"; }, [&]{
			auto iter = maker_list_.insert(std::make_pair(type, std::move(function)));
			if(!iter.second) throw std::logic_error("Module type '" + type + "' is double registered!");
		});
	}

	module_ptr disposer::make_module(std::string const& type, std::string const& chain, std::string const& name, io_list const& inputs, io_list const& outputs, parameter_processor&& parameters, bool is_start){
		auto iter = maker_list_.find(type);

		if(iter == maker_list_.end()){
			throw std::logic_error("Module '" + chain + "." + name + "': " + "Type '" + type + "' is unknown!");
		}

		try{
			auto result = iter->second(type, chain, name, inputs, outputs, parameters, is_start);
			for(auto const& param: parameters.unused_parameters()){
				log([&param, &chain, &name](log_base& os){ os << "In chain '" << chain << "' module '" << name << ": Unused parameter '" << param.first << "'='" << param.second << "'"; });
			}
			return result;
		}catch(std::exception const& error){
			throw std::runtime_error("Module '" + chain + "." + name + "': " + error.what());
		}
	}

	void disposer::load(std::string const& filename){
		auto config = parse(filename);

		check_semantic(config);
		unused_warnings(config);

		auto merged_config = merge(std::move(config));

		chain_list result;
		for(auto& chain: merged_config.chains){
			std::vector< module_ptr > modules;

			std::map< std::string, std::pair< output_base&, bool > > variables;

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

					variables.emplace(output.variable, std::pair< output_base&, bool >(entry->second, true));
				}

				first = false;
			}

			auto module_ptr_iter = modules.begin();
			for(auto& module: chain.modules){
				auto& module_ptr = *module_ptr_iter++;

				// module.outputs containes all active output names
				for(auto& output_name_and_var: module.outputs){
					auto output_iter = variables.find(output_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;

					if(output.active_types().empty()){
						std::ostringstream os;
						os
							<< "In chain '" << chain.name << "' module '" << module_ptr->name << "': Output '" + output_name_and_var.name
							<< "' (Variable: '" << output_name_and_var.variable << "') has no active output types";

						throw std::logic_error(os.str());
					}
				}

				// module.inputs containes all active inputs names
				for(auto& input_name_and_var: module.inputs){
					auto output_iter = variables.find(input_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;

					auto input_iter = module_ptr->inputs.find(input_name_and_var.name);
					assert(input_iter != module_ptr->inputs.end());

					auto& input = input_iter->second;

					if(!input.activate_types(output.active_types())){
						std::ostringstream os;
						os
							<< "In chain '" << chain.name << "' module '" << module_ptr->name << "': Variable '" + input_name_and_var.variable
							<< "' is incompatible with input '" << input_name_and_var.name << "'";

						os << " (active '" << input_name_and_var.variable << "' types: ";

						bool first = true;
						for(auto& type: output.active_types()){
							if(first){
								first = false;
							}else{
								os << ", ";
							}

							os << "'" << type.pretty_name() << "'";
						}

						os << "; possible '" << input_name_and_var.name << "' types: ";

						first = true;
						for(auto& type: input.types()){
							if(first){
								first = false;
							}else{
								os << ", ";
							}

							os << "'" << type.pretty_name() << "'";
						}

						os << ")";

						throw std::logic_error(os.str());
					}
				}
			}

			module_ptr_iter = modules.end();
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

					output.signal.connect(input, last_use);
					last_use = false;
				}
			}

			result.emplace(chain.name, ::disposer::chain(std::move(modules), chain.increase));
		}

		chains_ = std::move(result);
	}

	void disposer::trigger(std::string const& chain){
		auto iter = chains_.find(chain);
		if(iter == chains_.end()){
			throw std::logic_error("triggered chain '" + chain + "' does not exist");
		}
		iter->second.trigger();
	}

	std::unordered_set< std::string > disposer::chains()const{
		std::unordered_set< std::string > result;
		for(auto& chain: chains_) result.emplace(chain.first);
		return result;
	}



}
