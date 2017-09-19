//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__register_module__hpp_INCLUDED_
#define _disposer__core__register_module__hpp_INCLUDED_

#include "make_module.hpp"

#include <atomic>


namespace disposer{


	struct unit_test_key;

	/// \brief Hana Tag for \ref module_register_fn
	struct module_register_fn_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename Dimensions,
		typename Configuration,
		typename ModuleInitFn,
		typename ExecFn >
	class module_register_fn{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = module_register_fn_tag;

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		module_register_fn(
			dimension_list< Dimension ... > const& dims,
			module_configure< Config ... >&& list,
			module_init_fn< ModuleInitFn >&& module_init,
			exec_fn< ExecFn >&& exec
		)
			: called_flag_(false)
			, maker_{dims, std::move(list), module_init, exec}
			{}

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		module_register_fn(
			dimension_list< Dimension ... > const& dims,
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dims, std::move(list),
				module_init_fn< void >(), std::move(exec))
			{}

		/// \brief Constructor
		template < typename ... Config >
		module_register_fn(
			module_configure< Config ... >&& list,
			module_init_fn< ModuleInitFn >&& module_init,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				std::move(module_init), std::move(exec))
			{}

		/// \brief Constructor
		template < typename ... Config >
		module_register_fn(
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				module_init_fn< void >(), std::move(exec))
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
		module_maker< Dimensions, Configuration, ModuleInitFn, ExecFn > maker_;

		friend struct unit_test_key;
	};


	/// \brief Deduction guide
	template <
		typename ModuleInitFn,
		typename ExecFn,
		typename ... Dimension,
		typename ... Config >
	module_register_fn(
		dimension_list< Dimension ... > const& dims,
		module_configure< Config ... >&& list,
		module_init_fn< ModuleInitFn > const& module_init,
		exec_fn< ExecFn > const& exec
	) -> module_register_fn<
		dimension_list< Dimension ... >,
		module_configure< Config ... >,
		ModuleInitFn, ExecFn >;

}


#endif
