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
#include "../tool/ct_pretty_name.hpp"

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <boost/hana/type.hpp>
#include <boost/hana/traits.hpp>

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


		template < typename T, typename Accessory >
		static constexpr bool is_invocable_v()noexcept{
			return
				std::is_invocable_v< Fn const, hana::basic_type< T >,
					Accessory > ||
				std::is_invocable_v< Fn const, hana::basic_type< T > > ||
				std::is_invocable_v< Fn const >;
		}

		template < typename T, typename Accessory >
		static constexpr auto invoke_result_type()noexcept{
			static_assert(is_invocable_v< T, Accessory >(),
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R is void or convertible to T"
			);

			auto type = []{
					if constexpr(std::is_invocable_v< Fn const >){
						return hana::type_c< std::invoke_result_t< Fn const > >;
					}else if constexpr(
						std::is_invocable_v< Fn const, hana::basic_type< T > >
					){
						return hana::type_c< std::invoke_result_t< Fn const,
							hana::basic_type< T > > >;
					}else{
						return hana::type_c< std::invoke_result_t< Fn const,
							hana::basic_type< T >, Accessory > >;
					}
				}();

			auto const is_result_valid =
				hana::traits::is_void(type) ||
				hana::traits::is_convertible(type, hana::type_c< T >);
			static_assert(is_result_valid,
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R must be void or convertible to T"
			);

			return type;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename T, typename Accessory >
		static constexpr auto is_void_r(
			hana::basic_type< T >, Accessory
		)noexcept{
			return hana::traits::is_void(invoke_result_type< T, Accessory >());
		}

		template < typename T, typename Accessory >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(is_invocable_v< T, Accessory >(),
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R is void or convertible to T"
			);

			if constexpr(std::is_invocable_v< Fn const >){
				return std::is_nothrow_invocable_v< Fn const >;
			}else if constexpr(
				std::is_invocable_v< Fn const, hana::basic_type< T > >
			){
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T > >;
			}else{
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T >, Accessory >;
			}
		}

		/// \brief Operator for outputs
		template < typename T, typename Accessory >
		T operator()(
			std::string_view parameter_name,
			hana::basic_type< T > type,
			Accessory accessory
		)const noexcept(calc_noexcept< T, Accessory >()){
			return accessory.log(
				[parameter_name](logsys::stdlogb& os, T const* value){
					os << "parameter(" << parameter_name << ") ";
					if(value){
						os << "generated default value: ";
						print_if_supported(os, *value);
					}else{
						os << "no default value generated";
					}
					os << " [" << ct_pretty_name< T >() << "]";
				}, [&]()noexcept(calc_noexcept< T, Accessory >())->T{
					if constexpr(std::is_invocable_v< Fn const >){
						return std::invoke(fn_);
					}else if constexpr(
						std::is_invocable_v< Fn const, hana::basic_type< T > >
					){
						return std::invoke(fn_, type);
					}else{
						return std::invoke(fn_, type, accessory);
					}
				});
		}


	private:
		Fn fn_;
	};


	struct auto_default_t{
		template < typename T >
		void operator()(hana::basic_type< T >)const noexcept{}

		template < typename T >
		std::optional< T > operator()(
			hana::basic_type< std::optional< T > >
		)const noexcept{ return {}; }
	};

	constexpr auto auto_default = default_value_fn(auto_default_t{});

	template < typename T >
	constexpr auto default_value(T&& value)
	noexcept(std::is_nothrow_move_constructible_v< T >){
		return default_value_fn(
			[value = static_cast< T&& >(value)]()
			noexcept(std::is_nothrow_copy_constructible_v< T >)
			{
				return value;
			});
	}

	constexpr auto default_value()noexcept{
		return default_value_fn([](auto const type)
			noexcept(std::is_nothrow_default_constructible_v<
				typename decltype(type)::type >
			){
				static_assert(std::is_default_constructible_v<
					typename decltype(type)::type >,
					"type is not default constructible, you can't use "
					"default_value(), use default_value(value) instead");
				return typename decltype(type)::type();
			});
	}



}


#endif
