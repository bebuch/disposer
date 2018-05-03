//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component__hpp_INCLUDED_
#define _disposer__core__component__hpp_INCLUDED_

#include "component_base.hpp"
#include "component_init_fn.hpp"
#include "component_ref.hpp"
#include "module_name.hpp"

#include "../config/component_make_data.hpp"
#include "../config/validate_iop.hpp"


namespace disposer{


	/// \brief The actual component type
	template <
		typename TypeList,
		typename Parameters,
		typename ComponentInitFn >
	class component: public component_base{
	public:
		/// \brief Type for exec_fn
		using ref_type = component_init_ref< TypeList, Parameters >;

		/// \brief Actual user defined component object
		using state_type = std::invoke_result_t<
			component_init_fn< ComponentInitFn > const, ref_type >;


		/// \brief Constructor
		template < typename ... Ts, typename ... RefList >
		component(
			type_list< Ts ... >,
			std::string const& name,
			std::string const& type_name,
			system& system,
			hana::tuple< RefList ... >&& ref_list,
			component_init_fn< ComponentInitFn > const& component_fn
		)
			: component_base(name, type_name)
			, ref(type_list< Ts ... >{},
				state_, data_.parameters, name, system, location)
			, data_(std::move(ref_list))
			, state_(component_fn(ref_type(
				data_, this->name, system, location))) {}


		/// \brief Components are not copyable
		component(component const&) = delete;

		/// \brief Components are not movable
		component(component&&) = delete;


		/// \brief Ref object in component modules
		component_ref< TypeList, state_type, Parameters > ref;


	private:
		/// \brief shutdown component if it has a shutdown function
		///
		/// Every component which controls the disposer asynchronous should
		/// implement such a function.
		void shutdown()override{
			auto has_shutdown =
				hana::is_valid([](auto& t)->decltype((void)t.shutdown()){})
				(state_);
			if constexpr(has_shutdown){
				state_.shutdown();
			}
		}

		/// \brief Parameters
		component_data< TypeList, Parameters > data_;

		/// \brief The component object
		state_type state_;
	};

	template <
		typename ... Ts,
		typename ... RefList,
		typename ComponentInitFn >
	component(
		type_list< Ts ... >,
		std::string const&,
		std::string const&,
		system&,
		hana::tuple< RefList ... >&&,
		component_init_fn< ComponentInitFn > const&
	)
		-> component<
			type_list< Ts ... >,
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< parameter_tag >)),
			ComponentInitFn >;


}


#endif
