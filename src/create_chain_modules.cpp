#include <disposer/core/module_base.hpp>

#include <disposer/config/module_make_data.hpp>
#include <disposer/config/create_chain_modules.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <cassert>
#include <algorithm>


namespace disposer{ namespace{


	struct output_t{
		output_t(
			module_base const& module,
			output_base& ptr,
			std::size_t output_module_number
		)noexcept
			: module(module)
			, ptr(ptr)
			, output_module_number(output_module_number) {}

		module_base const& module;
		output_base& ptr;
		std::size_t output_module_number;
	};

	/// \brief Map from a variable name to an output
	using variables_map = std::map< std::string, output_t >;

	module_ptr create_module(
		module_maker_list const& module_makers,
		component_module_makers_list& component_module_makers,
		module_make_data const& data
	){
		auto const pos = data.type_name.find("//");
		if(pos == std::string::npos){
			auto iter = module_makers.find(data.type_name);

			if(iter == module_makers.end()){
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
		}else{
			auto const component_name = data.type_name.substr(0, pos);
			auto const module_type_name = data.type_name.substr(pos + 2);

			auto& modules = component_module_makers.at(component_name);
			auto const iter = modules.find(module_type_name);
			if(iter == modules.end()){
				throw std::logic_error(
					data.location() + "component module type(" + data.type_name
					+ ") is unknown!"
				);
			}

			auto& module = iter->second;
			try{
				return module.fn(data, module.usage_count);
			}catch(std::exception const& error){
				throw std::runtime_error(
					data.location() + error.what()
				);
			}
		}
	}

} }


namespace disposer{


	chain_module_list create_chain_modules(
		module_maker_list const& module_makers,
		component_module_makers_list& component_module_makers,
		types::embedded_config::chain const& config_chain
	){
		variables_map variables;

		chain_module_list result;

		for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
			auto const& config_module = config_chain.modules[i];

			logsys::log([&config_chain, &config_module, i](logsys::stdlogb& os){
				os << "chain(" << config_chain.name << ") module("
					<< i + 1 << ":" << config_module.type_name << ") created";
			}, [&]{
				// add the number of this module to all its wait_on modules
				for(auto wait_on: config_module.wait_ons){
					result.modules[wait_on].next_indexes.push_back(i);
				}

				// create input list
				input_list config_inputs;
				if(
					config_module.wait_ons.empty() &&
					config_module.inputs.empty()
				){
					// add as starter module
					result.start_indexes.push_back(i);
				}else{
					for(auto const& config_input: config_module.inputs){
						// find variable
						auto const iter = variables.find(config_input.variable);
						assert(iter != variables.end());

						// emplace input with connected output
						auto const& output_data = iter->second;
						auto const output_ptr = &(output_data.ptr);
						config_inputs.emplace(config_input.name, output_ptr);

						result.modules[output_data.output_module_number]
							.next_indexes.push_back(i);

						// remove config file variable if final use
						if(config_input.transfer == in_transfer::move){
							variables.erase(iter);
						}
					}
				}

				// create output list
				output_list config_outputs;
				for(auto const& config_output: config_module.outputs){
					config_outputs.emplace(
						config_output.name,
						config_output.use_count);
				}

				// create module (in a unique_ptr)
				result.modules.push_back(
					chain_module_data{create_module(
						module_makers,
						component_module_makers,
						{
							config_module.type_name,
							config_chain.name,
							i + 1,
							std::move(config_inputs),
							std::move(config_outputs),
							config_module.parameters
						}), config_module.inputs.size(), {}});

				// get a reference to the new module
				auto& module = *result.modules.back().module;

				// get outputs and add them to variables-output-map
				auto const output_map = module.output_name_to_ptr();
				for(auto const& config_output: config_module.outputs){
					variables.try_emplace(
						config_output.variable,
						module,
						*output_map.at(config_output.name),
						i
					);
				}
			});
		}

		assert(variables.empty());

		// remove duplicates from next_indexes
		for(auto& module: result.modules){
			auto const first = module.next_indexes.begin();
			auto const last = module.next_indexes.end();
			std::sort(first, last);
			auto const end = std::unique(first, last);
			module.next_indexes.erase(end, last);
			module.precursor_count -= last - end;
		}

		return result;
	}


}
