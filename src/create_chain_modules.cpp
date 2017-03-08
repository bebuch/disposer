#include <disposer/create_chain_modules.hpp>
#include <disposer/module_base.hpp>
#include <disposer/make_data.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


namespace disposer{


	creator_key make_creator_key(){
		return creator_key();
	}


}


namespace disposer{ namespace{


	/// \brief A reference to the output with last_use flag
	using output_pair = std::pair< output_base&, bool >;

	/// \brief Map from a variable name to an output
	using variables_map = std::unordered_map< std::string, output_pair >;


	auto find(module_base::input_list& container, std::string const& data){
		for(auto& value: container){
			if(value.get().name == data) return value;
		}
		throw std::logic_error("input '" + data + "' does not exist");
	}

	auto find(module_base::output_list& container, std::string const& data){
		for(auto& value: container){
			if(value.get().name == data) return value;
		}
		throw std::logic_error("output '" + data + "' does not exist");
	}


	module_ptr create_module(
		module_maker_list const& maker_list,
		make_data&& data
	){
		auto iter = maker_list.find(data.type_name);

		if(iter == maker_list.end()){
			throw std::logic_error(
				"Module '" + data.chain + "'.'" + data.name + "': "
				+ "Type '" + data.type_name + "' is unknown!"
			);
		}

		try{
			auto result = iter->second(data);
			for(auto const& param: data.params.unused()){
				log([&data, &param](log_base& os){
					os << "In chain '" << data.chain << "' module '"
						<< data.name << "': Unused data '" << param.first
						<< "'='" << param.second << "'"; });
			}
			return result;
		}catch(std::exception const& error){
			throw std::runtime_error(
				"Module '" + data.chain + "'.'" + data.name + "': "
				+ error.what()
			);
		}
	}

	auto create_modules(
		module_maker_list const& maker_list,
		types::merge::chain const& config_chain,
		variables_map& variables
	){
		std::vector< module_ptr > modules;

		for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
			auto& config_module = config_chain.modules[i];

			log([&config_module](log_base& os){
				os << "create module '" << config_module.module.first << "'";
			}, [&](){
				io_list config_inputs;
				for(auto& config_input: config_module.inputs){
					config_inputs.emplace(config_input.name);
				}

				io_list config_outputs;
				for(auto& config_output: config_module.outputs){
					config_outputs.emplace(config_output.name);
				}

				modules.push_back(create_module(maker_list, {
					config_module.module.second.type_name,
					config_chain.name,
					config_module.module.first,
					i,
					std::move(config_inputs),
					std::move(config_outputs),
					config_module.module.second.parameters
				}));

				auto& module = *modules.back();

				// save output variable names
				for(auto& config_output: config_module.outputs){
					auto& output = find(
							module.outputs(make_creator_key()),
							config_output.name
						).get();

					variables.emplace(
						config_output.variable,
						std::pair< output_base&, bool >(output, true)
					);
				}
			});
		}

		return modules;
	}

	void enable_output_types(
		types::merge::chain const& config_chain,
		std::vector< module_ptr > const& modules,
		variables_map const& variables
	){
		// go through all modules and enable output types in the target inputs
		auto module_ptr_iter = modules.begin();
		for(auto& config_module: config_chain.modules){
			auto& module = **module_ptr_iter++;

			log([&config_module](log_base& os){
				os << "enable input and output types in module '"
					<< config_module.module.first << "'";
			}, [&](){
				// config_module.inputs containes all enabled input names
				for(auto& config_input: config_module.inputs){
					auto output_iter = variables.find(config_input.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;
					auto& input = find(
							module.inputs(make_creator_key()),
							config_input.name
						).get();

					// try to enable the types from output in input
					if(!input.enable_types(
						make_creator_key(), output.enabled_types())
					){
						std::ostringstream os;
						os << "In chain '" << config_chain.name << "' module '"
							<< module.name << "': Variable '"
							<< config_input.variable
							<< "' is incompatible with input '"
							<< config_input.name << "'" << " (enabled '"
							<< config_input.variable << "' types: ";

						bool first = true;
						for(auto& type: output.enabled_types()){
							if(first){
								first = false;
							}else{
								os << ", ";
							}

							os << "'" << type.pretty_name() << "'";
						}

						os << "; possible '" << config_input.name
							<< "' types: ";

						first = true;
						for(auto& type: input.type_list()){
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

				log([&config_module](log_base& os){
					os << "call input_ready() in module '"
						<< config_module.module.first << "'";
				}, [&module](){
					module.input_ready(make_creator_key());
				});

				// config_module.outputs containes all enabled output names
				for(auto& config_output: config_module.outputs){
					auto output_iter = variables.find(config_output.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;

					if(output.enabled_types().empty()){
						std::ostringstream os;
						os << "In chain '" << config_chain.name << "' module '"
							<< module.name << "': Output '" + config_output.name
							<< "' (Variable: '" << config_output.variable
							<< "') has no enabled output types";

						throw std::logic_error(os.str());
					}
				}
			});
		}
	}

	void connect_modules(
		types::merge::chain const& config_chain,
		std::vector< module_ptr > const& modules,
		variables_map& variables
	){
		namespace adaptors = boost::adaptors;

		// go backward through all modules, because of the last_use information
		auto module_ptr_iter = modules.end();
		for(auto& config_module: adaptors::reverse(config_chain.modules)){
			--module_ptr_iter;
			auto& module = **module_ptr_iter;

			log([&config_module](log_base& os){
				os << "connect inputs of module '" << config_module.module.first
					<< "'";
			}, [&](){
				// config_module.inputs containes all enabled input names
				for(auto& config_input: config_module.inputs){
					auto output_iter = variables.find(config_input.variable);
					assert(output_iter != variables.end());

					auto& output = output_iter->second.first;
					auto& last_use = output_iter->second.second;

					auto& input = find(
							module.inputs(make_creator_key()),
							config_input.name
						).get();

					// connect input to output
					output.get_signal(make_creator_key())
						.connect(input, last_use);

					// the next one is no more the last use of the variable
					last_use = false;
				}
			});
		}
	}


} }


namespace disposer{


	std::vector< module_ptr > create_chain_modules(
		module_maker_list const& maker_list,
		types::merge::chain const& config_chain
	){
		variables_map variables;

		auto modules = create_modules(maker_list, config_chain, variables);

		enable_output_types(config_chain, modules, variables);

		connect_modules(config_chain, modules, variables);

		return modules;
	}


}
