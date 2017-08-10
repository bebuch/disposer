//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_maker__hpp_INCLUDED_
#define _disposer__core__module_maker__hpp_INCLUDED_

#include "module.hpp"

#include "../config/validate_iop.hpp"
#include "../config/module_make_data.hpp"

#include <boost/hana/tuple.hpp>

#include <atomic>


namespace disposer{


	/// \brief Maker function for \ref module in a std::unique_ptr
	template < typename MakerList, typename MakeData, typename StateMakerFn,
		typename ExecFn >
	auto make_module_ptr(
		MakerList const& maker_list,
		MakeData const& data,
		std::string_view location,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	){
		auto type = hana::unpack(maker_list, [](auto const& ... maker){
			return hana::type_c< hana::tuple<
				typename decltype(hana::typeid_(maker))::type::type ... > >;
		});

		using list_type = typename decltype(type)::type;

		return std::make_unique< module< list_type, StateMakerFn, ExecFn > >(
			maker_list, data, location, state_maker, exec);
	}


	/// \brief Provids types for constructing an module
	template < typename MakerList, typename StateMakerFn, typename ExecFn >
	struct module_maker{
		/// \brief Tuple of input/output/parameter-maker objects
		MakerList makers;

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
					location, makers, data.inputs);
				auto outputs = validate_iop< output_maker_tag >(
					location, makers, data.outputs);
				validate_iop< parameter_maker_tag >(
					location, makers, data.parameters);

				if(!inputs.empty() || !outputs.empty()){
					throw std::logic_error(location + "some inputs or "
						"outputs don't exist, see previos log messages for "
						"more details");
				}
			}

			std::string const basic_location = data.basic_location();

			// Create the module
			return make_module_ptr(
				makers, data, basic_location, state_maker, exec);
		}
	};


	/// \brief Wraps all given IOP configurations into a hana::tuple
	template < typename ... IOP_Makers >
	constexpr auto module_configure(IOP_Makers&& ... list){
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, IOP_Makers >(),
			hana::is_a< output_maker_tag, IOP_Makers >(),
			hana::is_a< parameter_maker_tag, IOP_Makers >()) ...),
			"at least one of the module configure arguments is not a disposer "
			"input, output or parameter maker");

		return hana::make_tuple(static_cast< IOP_Makers&& >(list) ...);
	}

	struct unit_test_key;

	/// \brief Hana Tag for \ref module_register_fn
	struct module_register_fn_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template < typename MakerList, typename StateMakerFn, typename ExecFn >
	class module_register_fn{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = module_register_fn_tag;

		/// \brief Constructor
		module_register_fn(
			MakerList&& list,
			state_maker_fn< StateMakerFn > const& state_maker,
			exec_fn< ExecFn > const& exec
		)
			: called_flag_(false)
			, maker_{static_cast< MakerList&& >(list), state_maker, exec}
			{}

		/// \brief Constructor
		module_register_fn(
			MakerList&& list,
			exec_fn< ExecFn > const& exec
		)
			: called_flag_(false)
			, maker_{static_cast< MakerList&& >(list),
				state_maker_fn< void >(), exec}
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
		module_maker< MakerList, StateMakerFn, ExecFn > maker_;

		friend struct unit_test_key;
	};


}


#endif
