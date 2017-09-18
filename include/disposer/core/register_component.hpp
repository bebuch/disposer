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
		typename MakerList,
		typename ComponentFn,
		typename ComponentModules >
	class component_register_fn{
	public:
		/// \brief Constructor
		component_register_fn(
			MakerList&& list,
			component_init< ComponentFn >&& component_fn,
			ComponentModules&& component_modules
		)
			: called_flag_(false)
			, maker_{
				static_cast< MakerList&& >(list),
				std::move(component_fn),
				static_cast< ComponentModules&& >(component_modules)
			}
			{}

		/// \brief Constructor
		component_register_fn(
			MakerList&& list,
			component_init< ComponentFn > const& component_fn,
			ComponentModules&& component_modules
		)
			: called_flag_(false)
			, maker_{
				static_cast< MakerList&& >(list),
				component_fn,
				static_cast< ComponentModules&& >(component_modules)
			}
			{}

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
		component_maker< MakerList, ComponentFn, ComponentModules > maker_;

		friend struct unit_test_key;
	};


}


#endif
