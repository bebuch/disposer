//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__verify_value_fn__hpp_INCLUDED_
#define _disposer__core__verify_value_fn__hpp_INCLUDED_

#include "../tool/ct_pretty_name.hpp"

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>


namespace disposer{


	struct verify_value_fn_tag;

	struct verify_value_always_t{};

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


		template < typename T, typename Ref >
		static constexpr bool calc_noexcept()noexcept{
			if constexpr(!std::is_same_v< Fn, verify_value_always_t >){
				static_assert(
					std::is_invocable_v< Fn const, T const& > ||
					std::is_invocable_v< Fn const, T const&, Ref >,
					"Wrong function signature for verify_value_fn, expected "
					"one of:\n"
					"  void function(T const& value)\n"
					"  void function(T const& value, auto ref)"
				);

				if constexpr(std::is_invocable_v< Fn const, T const& >){
					return std::is_nothrow_invocable_v< Fn const, T const& >;
				}else{
					return std::is_nothrow_invocable_v<
						Fn const, T const&, Ref >;
				}
			}else{
				return true;
			}
		}

		template < typename T, typename Ref >
		void operator()(
			std::string_view parameter_name,
			T const& value,
			Ref const& ref
		)const noexcept(calc_noexcept< T, Ref >()){
			(void)parameter_name; (void)value; (void)ref; // Silance GCC
			if constexpr(!std::is_same_v< Fn, verify_value_always_t >){
				ref.log([parameter_name](logsys::stdlogb& os){
						os << "parameter(" << parameter_name
							<< ") verified value of type ["
							<< ct_pretty_name< T >() << "]";
					}, [&]{
						if constexpr(std::is_invocable_v< Fn const, T const& >){
							std::invoke(fn_, value);
						}else{
							std::invoke(fn_, value, ref);
						}
					});
			}
		}

	private:
		Fn fn_;
	};

	constexpr auto verify_value_always =
		verify_value_fn< verify_value_always_t >{};


}


#endif
