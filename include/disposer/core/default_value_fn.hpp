//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__default_value_fn__hpp_INCLUDED_
#define _disposer__core__default_value_fn__hpp_INCLUDED_

#include "../tool/print_if_supported.hpp"

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <boost/hana/type.hpp>

#include <optional>


namespace disposer{


	struct default_value_fn_tag;

	template < typename Fn >
	class default_value_fn{
	public:
		using hana_tag = default_value_fn_tag;

		constexpr default_value_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit default_value_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit default_value_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T, typename ... Ts >
		static constexpr bool is_invocable_v()noexcept{
			return std::is_invocable_v< Fn const, Accessory const&,
				hana::basic_type< T >, hana::basic_type< Ts > ... >;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T, typename ... Ts >
		static constexpr bool is_void_r_v()noexcept{
			if constexpr(
				is_invocable_v< Accessory, T, Ts ... >()
			){
				return std::is_void_v< std::invoke_result_t< Fn const,
					Accessory const&, hana::basic_type< T >,
					hana::basic_type< Ts > ... > >;
			}else{
				return false;
			}
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T, typename ... Ts >
		static constexpr auto is_void_r(
			Accessory const&, hana::basic_type< T >, hana::basic_type< Ts > ...
		)noexcept{
			if constexpr(is_void_r_v< Accessory, T, Ts ... >()){
				return hana::true_c;
			}else{
				return hana::false_c;
			}
		}

		template < typename Accessory, typename T, typename ... Ts >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(is_invocable_v< Accessory, T, Ts ... >(),
				"Wrong function signature, expected: "
				"U f(auto const& iop, hana::basic_type< T > type, "
				"hana::basic_type< Ts > ... type_dependancies) where U is "
				"void or convertible to T"
			);

			static_assert(std::is_invocable_r_v< T, Fn const,
				Accessory const&, hana::basic_type< T >,
				hana::basic_type< Ts > ... >
				|| is_void_r_v< Accessory, T, Ts ... >(),
				"Result of default_value_fn is neither void nor convertible "
				"to T"
			);

			return std::is_nothrow_invocable_v< Fn const,
				Accessory const&, hana::basic_type< T >,
				hana::basic_type< Ts > ... >;
		}

		/// \brief Operator for outputs
		template < typename Accessory, typename T, typename ... Ts >
		T operator()(
			Accessory const& accessory,
			hana::basic_type< T > type,
			hana::basic_type< Ts > ... type_dependancies
		)const noexcept(
			calc_noexcept< Accessory, T, Ts ... >()
		){
			return accessory.log([](logsys::stdlogb& os, T const* value){
					if(value){
						os << "generated default value: ";
						print_if_supported(os, *value);
					}else{
						os << "no default value generated";
					}
					os << " [" << type_index::type_id< T >().pretty_name()
						<< "]";
				}, [&]()noexcept(
					calc_noexcept< Accessory, T, Ts ... >()
				)->T{
					return std::invoke(fn_, accessory, type,
						type_dependancies ...);
				});
		}


	private:
		Fn fn_;
	};


	struct auto_default_t{
		template < typename Accessory, typename T, typename ... Ts >
		void operator()(
			Accessory const&,
			hana::basic_type< T >,
			hana::basic_type< Ts > ...
		)const noexcept{}

		template < typename Accessory, typename T, typename ... Ts >
		std::optional< T > operator()(
			Accessory const&,
			hana::basic_type< std::optional< T > >,
			hana::basic_type< Ts > ...
		)const noexcept{ return {}; }
	};

	constexpr auto auto_default = default_value_fn(auto_default_t{});

	template < typename T >
	auto default_value(T&& value)
	noexcept(std::is_nothrow_move_constructible_v< T >){
		return default_value_fn(
			[value = std::move(value)](auto const&, auto, auto ...)
			noexcept(std::is_nothrow_copy_constructible_v< T >)
			{ return value; });
	}




}


#endif
