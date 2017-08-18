//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_module__hpp_INCLUDED_
#define _disposer__core__make_module__hpp_INCLUDED_

#include "module.hpp"

#include "../config/validate_iop.hpp"
#include "../config/module_make_data.hpp"

#include <atomic>


namespace disposer{


	/// \brief Maker function for \ref module in a std::unique_ptr
	template <
		typename ... Dimension,
		typename ... Config,
		typename StateMakerFn,
		typename ExecFn >
	auto make_module_ptr(
		dimension_list< Dimension ... > const& dims
		module_configure< Config ... > const& config,
		module_make_data const& data,
		std::string_view location,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	){
		return std::make_unique< module< list_type, StateMakerFn, ExecFn > >(
			dims, config, data, location, state_maker, exec);
	}


	/// \brief Provids types for constructing an module
	template <
		typename Dimensions,
		typename Configuration,
		typename StateMakerFn,
		typename ExecFn >
	struct module_maker{
		/// \brief An dimension_list object
		Dimensions dimensions;

		/// \brief Tuple of input/output/parameter-maker objects
		Configuration configuration;

		/// \brief The function object that is called in enable()
		state_maker_fn< StateMakerFn > state_maker;

		/// \brief The function object that is called in exec()
		exec_fn< ExecFn > exec;


		/// \brief Create an module object
		auto operator()(module_make_data const& data)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto const location = data.location();
			{
				auto inputs = validate_iop< input_maker_tag >(
					location, configuration, data.inputs);
				auto outputs = validate_iop< output_maker_tag >(
					location, configuration, data.outputs);
				validate_iop< parameter_maker_tag >(
					location, configuration, data.parameters);

				if(!inputs.empty() || !outputs.empty()){
					throw std::logic_error(location + "some inputs or "
						"outputs don't exist, see previos log messages for "
						"more details");
				}
			}

			std::string const basic_location = data.basic_location();

			// Create the module
			return make_module_ptr(
				configuration, data, basic_location, state_maker, exec);
		}
	};


	/// \brief The list of module configuration items
	template < typename ... Config >
	struct module_configure{
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, Config >(),
			hana::is_a< output_maker_tag, Config >(),
			hana::is_a< parameter_maker_tag, Config >(),
			hana::is_a< set_dimension_fn_tag, Config >()) ...),
			"at least one of the module configure arguments is not "
			"an input maker, an output maker, a parameter maker or "
			"a set_dimension_fn");

		/// \brief The data
		hana::tuple< Config > config_list;

		/// \brief Constructor
		constexpr auto module_configure(Config&& ... list)
			: config_list(static_cast< Config&& >(list) ...) {}
	};


	/// \brief Deduction guide to store all config items as values
	template < typename ... Config >
	module_configure(Config&& ...) -> module_configure
		< std::remove_cv_t< std::remove_reference_t< Config > > ... >;

	struct unit_test_key;

	/// \brief Hana Tag for \ref module_register_fn
	struct module_register_fn_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename Dimensions,
		typename Configuration,
		typename StateMakerFn,
		typename ExecFn >
	class module_register_fn{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = module_register_fn_tag;

		/// \brief Constructor
		template < typename Dimension ..., typename Config ... >
		module_register_fn(
			dimension_list< Dimension ... >&& dims
			module_configure< Config ... >&& list,
			state_maker_fn< StateMakerFn >&& state_maker,
			exec_fn< ExecFn >&& exec
		)
			: called_flag_(false)
			, maker_{dims, static_cast< MakerList&& >(list), state_maker, exec}
			{}

		/// \brief Constructor
		template < typename Dimension ..., typename Config ... >
		module_register_fn(
			dimension_list< Dimension ... >&& dims
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(std::move(dims), std::move(list),
				state_maker_fn< void >(), std::move(exec))
			{}

		/// \brief Constructor
		template < typename Config ... >
		module_register_fn(
			module_configure< Config ... >&& list,
			state_maker_fn< StateMakerFn >&& state_maker,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				std::move(state_maker), std::move(exec))
			{}

		/// \brief Constructor
		template < typename Config ... >
		module_register_fn(
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				state_maker_fn< void >(), std::move(exec))
			{}

		/// \brief Call this function to register the module with the given type
		///        name via the given module_declarant
		void operator()(std::string const& module_type, module_declarant& add){
			if(!called_flag_.exchange(true)){
				add(module_type,
					[maker{std::move(maker_)}](module_make_data const& data){
						return maker(data);
					});
			}else{
				throw std::runtime_error("called module register function '"
					+ module_type + "' more than once");
			}
		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The module_maker object
		module_maker< Dimensions, Configuration, StateMakerFn, ExecFn > maker_;

		friend struct unit_test_key;
	};


	/// \brief Deduction guide
	template <
		typename StateMakerFn,
		typename ExecFn,
		typename Dimension ...,
		typename Config ... >
	module_register_fn(
		dimension_list< Dimension ... > const& dims
		module_configure< Config ... >&& list,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	) -> module_register_fn<
		dimension_list< Dimension ... >,
		module_configure< Config ... >,
		StateMakerFn, ExecFn >;

}


#endif
