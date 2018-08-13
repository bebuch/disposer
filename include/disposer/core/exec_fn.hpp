//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_fn__hpp_INCLUDED_
#define _disposer__core__exec_fn__hpp_INCLUDED_

#include "module_ref.hpp"


namespace disposer{


	/// \brief Wrapper for the module exec function
	template < typename Fn >
	class exec_fn{
	public:
		constexpr exec_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit exec_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit exec_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template <
			typename TypeList,
			typename State,
			typename ExecInputs,
			typename ExecOutputs,
			typename Parameters,
			typename Component >
		void operator()(
			module_ref< TypeList, State, ExecInputs, ExecOutputs,
				Parameters, Component >& ref
		){
			// TODO: calulate noexcept
			if constexpr(
				std::is_invocable_v< Fn, module_ref< TypeList, State,
					ExecInputs, ExecOutputs, Parameters, Component >& >
			){
				std::invoke(fn_, ref);
			}else if constexpr(std::is_invocable_v< Fn >){
				(void)ref; // silance GCC
				std::invoke(fn_);
			}else{
				static_assert(detail::false_c< State >,
					"Fn must be invokable with module_ref& or "
					"without arguments");
			}
		}


	private:
		Fn fn_;
	};


}


#endif
