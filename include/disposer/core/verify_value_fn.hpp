//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__verify_value_fn__hpp_INCLUDED_
#define _disposer__core__verify_value_fn__hpp_INCLUDED_

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>


namespace disposer{


	struct verify_value_fn_tag;

	struct verify_value_always{};

	template < typename Fn >
	class verify_value_fn{
	public:
		using hana_tag = verify_value_fn_tag;

		constexpr verify_value_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit verify_value_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit verify_value_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename T >
		static constexpr bool calc_noexcept()noexcept{
			if constexpr(!std::is_same_v< Fn, verify_value_always >){
				static_assert(std::is_invocable_v< Fn const, T >,
					"Wrong function signature, expected: void f(auto value)");

				return noexcept(
					std::declval< Fn const >()(std::declval< T >()));
			}else{
				return false;
			}
		}

		template < typename T >
		void operator()(std::string_view location, T const& value)const
		noexcept(calc_noexcept< T >()){
			if constexpr(!std::is_same_v< Fn, verify_value_always >){
				logsys::log([location](logsys::stdlogb& os){
						os << location << "verified value of type ["
							<< type_index::type_id< T >().pretty_name() << "]";
					}, [&]{ std::invoke(fn_, value); });
			}
		}

	private:
		Fn fn_;
	};


}


#endif
