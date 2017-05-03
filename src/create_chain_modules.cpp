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
	using variables_map = std::map< std::string, output_base* >;

	module_ptr create_module(
		module_maker_list const& maker_list,
		make_data&& data
	){
		auto iter = maker_list.find(data.type_name);

		if(iter == maker_list.end()){
			throw std::logic_error(
				data.location() + "type '" + data.type_name + "' is unknown!"
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
		types::merge::chain const& config_chain
	){
		variables_map variables;

		std::vector< module_ptr > modules;

		for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
			auto& config_module = config_chain.modules[i];

			logsys::log([&config_module](logsys::stdlogb& os){
				os << "create module '" << config_module.type_name << "'";
			}, [&](){
				// create input list
				input_list config_inputs;
				for(auto& config_input: config_module.inputs){
					auto output_ptr = variables[config_input.variable];
					config_inputs.emplace(config_input.name, output_ptr);
				}

				// create output list
				output_list config_outputs;
				for(auto& config_output: config_module.outputs){
					config_outputs.emplace(config_output.name);
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

				// save output variables
				auto output_map = module.get_outputs(make_creator_key());
				for(auto& config_output: config_module.outputs){
					variables.emplace(
						config_output.variable,
						output_map[config_output.name]
					);
				}
			});
		}

		return modules;
	}


}
