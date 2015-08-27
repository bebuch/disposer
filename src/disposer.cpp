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


	void disposer::add_module_maker(std::string const& type_name, maker_function&& function){
		log([&type_name](log_base& os){ os << "register module type name '" << type_name << "'"; }, [&]{
			auto iter = maker_list_.insert(std::make_pair(type_name, std::move(function)));
			if(!iter.second) throw std::logic_error("Module type name '" + type_name + "' is double registered!");
		});
	}

	module_ptr disposer::make_module(make_data&& data){
		auto iter = maker_list_.find(data.type_name);

		if(iter == maker_list_.end()){
			throw std::logic_error("Module '" + data.chain + "." + data.name + "': " + "Type '" + data.type_name + "' is unknown!");
		}

		try{
			auto result = iter->second(data);
			for(auto const& param: data.params.unused()){
				log([&data, &param](log_base& os){ os << "In chain '" << data.chain << "' module '" << data.name << ": Unused data '" << param.first << "'='" << param.second << "'"; });
			}
			return result;
		}catch(std::exception const& error){
			throw std::runtime_error("Module '" + data.chain + "." + data.name + "': " + error.what());
		}
	}

	void disposer::load(std::string const& filename){
		auto config = parse(filename);

		check_semantic(config);
		unused_warnings(config);

		auto merged_config = merge(std::move(config));

		auto find = [](auto& container, std::string const& data){
			for(auto& value: container){
				if(value.get().name == data) return value;
			}
			throw std::logic_error("programming error in disposer: '" + data + "' is not in input or output");
		};

		std::unordered_map< std::string, chain > chains;
		std::unordered_map< std::string, id_generator > id_generators;
		for(auto& config_chain: merged_config.chains){
			std::vector< module_ptr > modules;

			std::map< std::string, std::pair< output_base&, bool > > variables;

			for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
				auto& config_module = config_chain.modules[i];

				io_list config_inputs;
				for(auto& config_input: config_module.inputs){
					config_inputs.emplace(config_input.name);
				}

				io_list config_outputs;
				for(auto& config_output: config_module.outputs){
					config_outputs.emplace(config_output.name);
				}

				modules.push_back(make_module({
					config_module.module.second.type_name,
					config_chain.name,
					config_module.module.first,
					i,
					std::move(config_inputs),
					std::move(config_outputs),
					config_module.module.second.parameters
				}));

				auto& module = *modules.back();

				for(auto& config_output: config_module.outputs){
					auto& output = find(module.outputs_, config_output.name).get();

					variables.emplace(config_output.variable, std::pair< output_base&, bool >(output, true));
				}
			}

			auto module_ptr_iter = modules.begin();
			for(auto& config_module: config_chain.modules){
				auto& module_ptr = *module_ptr_iter++;

				// module.outputs containes all active output names
				for(auto& output_name_and_var: config_module.outputs){
					auto output_iter = variables.find(output_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;

					if(output.active_types().empty()){
						std::ostringstream os;
						os
							<< "In chain '" << config_chain.name << "' module '" << module_ptr->name << "': Output '" + output_name_and_var.name
							<< "' (Variable: '" << output_name_and_var.variable << "') has no active output types";

						throw std::logic_error(os.str());
					}
				}

				// module.inputs containes all active inputs names
				for(auto& input_name_and_var: config_module.inputs){
					auto output_iter = variables.find(input_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;
					auto& input = find(module_ptr->inputs_, input_name_and_var.name).get();

					if(!input.activate_types(output.active_types())){
						std::ostringstream os;
						os
							<< "In chain '" << config_chain.name << "' module '" << module_ptr->name << "': Variable '" + input_name_and_var.variable
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
			for(auto& config_module: boost::adaptors::reverse(config_chain.modules)){
				--module_ptr_iter;
				auto& module_ptr = *module_ptr_iter;

				for(auto& input_name_and_var: config_module.inputs){
					auto output_iter = variables.find(input_name_and_var.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;
					auto& last_use = output_iter->second.second;

					auto& input = find(module_ptr->inputs_, input_name_and_var.name).get();

					output.signal.connect(input, last_use);
					last_use = false;
				}
			}

			chains.emplace(std::piecewise_construct, std::make_tuple(config_chain.name), std::forward_as_tuple(std::move(modules), id_generators[config_chain.id_generator], config_chain.increase));
		}

		chains_ = std::move(chains);
		id_generators_ = std::move(id_generators);
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
