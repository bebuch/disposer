//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__register_component__hpp_INCLUDED_
#define _disposer__core__register_component__hpp_INCLUDED_

#include "make_component.hpp"

#include <atomic>



namespace disposer{


	struct unit_test_key;

	/// \brief Registers a component configuration in the \ref disposer
	template <
		typename DimensionList,
		typename Configuration,
		typename Modules,
		typename ComponentInitFn >
	class component_register_fn{
	public:
		/// \brief Constructor
		template <
			typename ... Dimension,
			typename ... Config,
			typename ... Module >
		component_register_fn(
			dimension_list< Dimension ... > dims,
			component_configure< Config ... > list,
			component_init_fn< ComponentInitFn > module_init,
			component_modules< Module ... > modules
		)
			: called_flag_(false)
			, maker_{
				dims,
				std::move(list),
				std::move(modules),
				std::move(module_init)
			} {}

		/// \brief Constructor
		template < typename ... Config, typename ... Module >
		component_register_fn(
			component_configure< Config ... > list,
			component_init_fn< ComponentInitFn > module_init,
			component_modules< Module ... > modules
		)
			: component_register_fn{dimension_list{}, std::move(list),
				std::move(module_init), std::move(modules)} {}

		/// \brief Constructor
		template < typename ... Dimension, typename ... Module >
		component_register_fn(
			dimension_list< Dimension ... > dims,
			component_init_fn< ComponentInitFn > module_init,
			component_modules< Module ... > modules
		)
			: component_register_fn{dims, component_configure<>{},
				std::move(module_init), std::move(modules)} {}

		/// \brief Constructor
		template < typename ... Module >
		component_register_fn(
			component_init_fn< ComponentInitFn > module_init,
			component_modules< Module ... > modules
		)
			: component_register_fn{dimension_list{}, component_configure<>{},
				std::move(module_init), std::move(modules)} {}


		/// \brief Call this function to register the component with the given
		///        type name via the given component_declarant
		void operator()(
			std::string const& component_type,
			component_declarant& add
		){
			if(!called_flag_.exchange(true)){
				add(component_type,
					[maker{std::move(maker_)}, &add]
					(component_make_data const& data){
						return maker(data, add.disposer());
					});
			}else{
				throw std::runtime_error("called register function '"
					+ component_type + "' more than once");
			}

		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The component_maker object
		component_maker<
			DimensionList, Configuration, Modules, ComponentInitFn > maker_;

		friend struct unit_test_key;
	};


	template <
		typename ... Dimension,
		typename ... Config,
		typename ... Module,
		typename ComponentInitFn >
	component_register_fn(
		dimension_list< Dimension ... > dims,
		component_configure< Config ... > list,
		component_init_fn< ComponentInitFn > module_init,
		component_modules< Module ... > modules
	) -> component_register_fn<
			dimension_list< Dimension ... >,
			component_configure< Config ... >,
			component_modules< Module ... >,
			ComponentInitFn >;

	/// \brief Constructor
	template <
		typename ... Config,
		typename ... Module,
		typename ComponentInitFn >
	component_register_fn(
		component_configure< Config ... > list,
		component_init_fn< ComponentInitFn > module_init,
		component_modules< Module ... > modules
	) -> component_register_fn<
			dimension_list<>,
			component_configure< Config ... >,
			component_modules< Module ... >,
			ComponentInitFn >;

	/// \brief Constructor
	template <
		typename ... Dimension,
		typename ... Module,
		typename ComponentInitFn >
	component_register_fn(
		dimension_list< Dimension ... > dims,
		component_init_fn< ComponentInitFn > module_init,
		component_modules< Module ... > modules
	) -> component_register_fn<
			dimension_list< Dimension ... >,
			component_configure<>,
			component_modules< Module ... >,
			ComponentInitFn >;

	/// \brief Constructor
	template <
		typename ... Module,
		typename ComponentInitFn >
	component_register_fn(
		component_init_fn< ComponentInitFn > module_init,
		component_modules< Module ... > modules
	) -> component_register_fn<
			dimension_list<>,
			component_configure<>,
			component_modules< Module ... >,
			ComponentInitFn >;


}


#endif
