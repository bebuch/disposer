#include <disposer/core/module_base.hpp>

#include <disposer/config/module_make_data.hpp>
#include <disposer/config/create_chain_modules.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <cassert>


namespace disposer{ namespace{


	struct output_t{
		output_t(
			module_base const& module,
			output_base& output
		)noexcept
			: module(module)
			, output(output) {}

// 		output_t(output_t const& other)noexcept
// 			: module(other.module)
// 			, output(other.output) {}

		module_base const& module;
		output_base& output;
	};

	/// \brief Map from a variable name to an output
	using variables_map = std::map< std::string, output_t >;

	module_ptr create_module(
		module_maker_list const& maker_list,
		module_make_data const& data
	){
		auto iter = maker_list.find(data.type_name);

		if(iter == maker_list.end()){
			throw std::logic_error(
				data.location() + "module type(" + data.type_name
				+ ") is unknown!"
			);
		}

		try{
			return iter->second(data);
		}catch(std::exception const& error){
			throw std::runtime_error(
				data.location() + error.what()
			);
		}
	}


} }


namespace disposer{


	std::vector< module_ptr > create_chain_modules(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain
	){
		variables_map variables;

		std::vector< module_ptr > modules;

		for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
			auto const& config_module = config_chain.modules[i];

			logsys::log([&config_chain, &config_module, i](logsys::stdlogb& os){
				os << "chain(" << config_chain.name << ") module("
					<< i + 1 << ":" << config_module.type_name << ") created";
			}, [&]{
				// create input list
				input_list config_inputs;
				for(auto const& config_input: config_module.inputs){
					// find variable
					auto const iter = variables.find(config_input.variable);
					assert(iter != variables.end());

					// emplace input with connected output
					auto const output_ptr = &(iter->second.output);
					config_inputs.emplace(config_input.name, output_ptr);

					// remove config file variable if final use
					if(config_input.transfer == in_transfer::move){
						variables.erase(iter);
					}
				}

				// create output list
				output_list config_outputs;
				for(auto const& config_output: config_module.outputs){
					config_outputs.emplace(config_output.name, 0);
				}

				// create module (in a unique_ptr)
				modules.push_back(create_module(maker_list, {
					config_module.type_name,
					config_chain.name,
					i + 1,
					std::move(config_inputs),
					std::move(config_outputs),
					config_module.parameters
				}));

				// get a reference to the new module
				auto& module = *modules.back();

				// get outputs and add them to variables-output-map
				auto const output_map = module.output_name_to_ptr();
				for(auto const& config_output: config_module.outputs){
					variables.try_emplace(
						config_output.variable,
						module,
						*output_map.at(config_output.name)
					);
				}
			});
		}

		assert(variables.empty());

		return modules;
	}


}
