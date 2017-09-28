//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__register_fn__hpp_INCLUDED_
#define _disposer__core__register_fn__hpp_INCLUDED_

#include "component_accessory.hpp"


namespace disposer{


	struct module_register_fn_tag;

	template < typename Fn >
	class register_fn{
	public:
		constexpr register_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

 		constexpr explicit register_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

 		constexpr explicit register_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}

		template < typename TypeList, typename State, typename Parameters >
		auto operator()(
			component_accessory< TypeList, State, Parameters >& accessory
		)const{
			// TODO: calulate noexcept
			if constexpr(std::is_invocable_v< Fn const,
				component_accessory< TypeList, State, Parameters >& >
			){
				static_assert(hana::is_a< module_register_fn_tag,
						std::invoke_result_t< Fn const, component_accessory<
							TypeList, State, Parameters >& > >(),
					"register_fn Fn must return a module_register_fn");
				return std::invoke(fn_, accessory);
			}else{
				static_assert(detail::false_c< Fn >,
					"Fn function must be const invokable with "
					"component_accessory");
			}
		}

	private:
		Fn fn_;
	};


}


#endif
