#include <disposer/create_chain_modules.hpp>
#include <disposer/module_base.hpp>
#include <disposer/make_data.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


namespace disposer{


	struct creator_key;

	/// \brief Defined in create_chain_modules.cpp
	creator_key make_creator_key();

	/// \brief Access key creator functions
	struct creator_key{
	private:
		/// \brief Constructor
		constexpr creator_key()noexcept = default;
		friend creator_key make_creator_key();
	};

	creator_key make_creator_key(){
		return creator_key();
	}


}


namespace disposer{ namespace{


	/// \brief Map from a variable name to an output
	using variables_map = std::unordered_map< std::string, output_base* >;


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
				"Module '" + data.chain + "'.'" + std::to_string(data.number)
				+ "': " + "Type '" + data.type_name + "' is unknown!"
			);
		}

		try{
			auto result = iter->second(data);
			for(auto const& param: data.params.unused()){
				logsys::log([&data, &param](logsys::stdlogb& os){
					os << "In chain '" << data.chain << "' module '"
						<< data.number << "': Unused data '" << param.first
						<< "'='" << param.second << "'"; });
			}
			return result;
		}catch(std::exception const& error){
			throw std::runtime_error(
				"Module '" + data.chain + "'.'" + std::to_string(data.number)
				+ "': " + error.what()
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

			logsys::log([&config_module](logsys::stdlogb& os){
				os << "create module '" << config_module.type_name << "'";
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
					config_module.type_name,
					config_chain.name,
					i,
					std::move(config_inputs),
					std::move(config_outputs),
					config_module.parameters
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
						&output
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

			logsys::log([&config_module](logsys::stdlogb& os){
				os << "enable input and output types in module '"
					<< config_module.type_name << "'";
			}, [&](){
				// config_module.inputs containes all enabled input names
				for(auto& config_input: config_module.inputs){
					auto output_iter = variables.find(config_input.variable);
					assert(output_iter != variables.end());

					auto output_ptr = output_iter->second;
					auto& input = find(
							module.inputs(make_creator_key()),
							config_input.name
						).get();

					// try to enable the types from output in input
					if(!input.enable_types(
						make_creator_key(), output_ptr->enabled_types())
					){
						std::ostringstream os;
						os << "In chain '" << config_chain.name << "' module '"
							<< std::to_string(module.number) << "': Variable '"
							<< config_input.variable
							<< "' is incompatible with input '"
							<< config_input.name << "'" << " (enabled '"
							<< config_input.variable << "' types: ";

						bool first = true;
						for(auto& type: output_ptr->enabled_types()){
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

				logsys::log([&config_module](logsys::stdlogb& os){
					os << "call input_ready() in module '"
						<< config_module.type_name << "'";
				}, [&module](){
					module.input_ready(make_creator_key());
				});

				// config_module.outputs containes all enabled output names
				for(auto& config_output: config_module.outputs){
					auto output_iter = variables.find(config_output.variable);
					assert(output_iter != variables.end());

					auto output_ptr = output_iter->second;

					if(output_ptr->enabled_types().empty()){
						std::ostringstream os;
						os << "In chain '" << config_chain.name << "' module '"
							<< module.number << "': Output '"
							<< config_output.name << "' (Variable: '"
							<< config_output.variable
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
		assert(modules.size() == config_chain.modules.size());
		auto config_iter = config_chain.modules.begin();
		for(auto& module: modules){
			auto& config_module = *config_iter++;

			logsys::log([&config_module](logsys::stdlogb& os){
				os << "connect inputs of module '"
					<< config_module.type_name
					<< "'";
			}, [&module, &config_module, &variables]{
				// config_module.inputs containes all enabled input names
				for(auto& config_input: config_module.inputs){
					auto output_iter = variables.find(config_input.variable);
					assert(output_iter != variables.end());

					auto output_ptr = output_iter->second;

					auto& input = find(
							module->inputs(make_creator_key()),
							config_input.name
						).get();

					// connect input to output
					input.set_output(make_creator_key(), output_ptr);
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
