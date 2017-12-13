//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__verify_fn__hpp_INCLUDED_
#define _disposer__core__verify_fn__hpp_INCLUDED_

#include <logsys/stdlogb.hpp>


namespace disposer{


	struct verify_fn_tag;

	template < typename Fn >
	class verify_fn{
	public:
		using hana_tag = verify_fn_tag;

		constexpr verify_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit verify_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit verify_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(std::is_invocable_v< Fn const, Accessory >,
				"Wrong function signature for verify_fn, expected:\n"
				"void function(auto accessory)");

			return std::is_nothrow_invocable_v< Fn const, Accessory >;
		}

		template < typename Accessory >
		void operator()(Accessory const& accessory)const
		noexcept(calc_noexcept< Accessory >()){
			accessory.log([](logsys::stdlogb& os){
					os << "verfiy";
				}, [&]{ std::invoke(fn_, accessory); });
		}

	private:
		Fn fn_;
	};


}


#endif
