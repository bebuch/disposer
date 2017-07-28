#include <disposer/create_chain_modules.hpp>
#include <disposer/module_base.hpp>
#include <disposer/module_make_data.hpp>

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


	struct output_t{
		constexpr output_t(
			module_base const& module,
			output_base& output
		)noexcept
			: module(module)
			, output(output) {}

		constexpr output_t(output_t const& other)noexcept
			: module(other.module)
			, output(other.output) {}

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
		types::merge::chain const& config_chain
	){
		variables_map variables;

		std::vector< module_ptr > modules;

		for(std::size_t i = 0; i < config_chain.modules.size(); ++i){
			auto const& config_module = config_chain.modules[i];

			logsys::log([&config_chain, &config_module, i](logsys::stdlogb& os){
				os << "chain(" << config_chain.name << ") module("
					<< i + 1 << ':' << config_module.type_name << ") created";
			}, [&](){
				// create input list
				input_list config_inputs;
				for(auto const& config_input: config_module.inputs){
					bool const last_use =
						config_input.transfer == in_transfer::move;
					auto const output_ptr =
						&variables.at(config_input.variable).output;
					config_inputs.try_emplace(
						config_input.name, output_ptr, last_use);
					if(last_use) variables.erase(config_input.variable);
				}

				// create output list
				output_list config_outputs;
				for(auto const& config_output: config_module.outputs){
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
				auto const output_map = module.get_outputs(make_creator_key());
				for(auto const& config_output: config_module.outputs){
					variables.try_emplace(
						config_output.variable,
						module,
						*output_map.at(config_output.name)
					);
				}
			});
		}

		if(variables.empty()) return modules;

		std::ostringstream os;
		os << "Some variables are never finally used: ";
		bool first = true;
		for(auto const& [name, data]: variables){
			if(first){ first = false; }else{ os << ", "; }
			os << "variable(" << name << ") from chain(" << data.module.chain
				<< ") module(" << data.module.number << ":"
				<< data.module.type_name << ") output("
				<< data.output.get_name() << ')';
		}

		throw std::logic_error(os.str());
	}


}
