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


		template < typename Accessory, typename T >
		static constexpr bool is_invocable_v()noexcept{
			return std::is_invocable_v< Fn const, Accessory const&,
				hana::basic_type< T > >;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T >
		static constexpr bool is_void_r_v()noexcept{
			if constexpr(is_invocable_v< Accessory, T >()){
				return std::is_void_v< std::invoke_result_t< Fn const,
					Accessory const&, hana::basic_type< T > > >;
			}else{
				return false;
			}
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T >
		static constexpr auto is_void_r(
			Accessory const&, hana::basic_type< T >
		)noexcept{
			if constexpr(is_void_r_v< Accessory, T >()){
				return hana::true_c;
			}else{
				return hana::false_c;
			}
		}

		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(is_invocable_v< Accessory, T >(),
				"Wrong function signature, expected: "
				"U f(auto const& iop, hana::basic_type< T > type) where U is "
				"void or convertible to T"
			);

			static_assert(std::is_invocable_r_v< T, Fn const,
				Accessory const&, hana::basic_type< T > >
				|| is_void_r_v< Accessory, T >(),
				"Result of default_value_fn is neither void nor convertible "
				"to T"
			);

			return std::is_nothrow_invocable_v< Fn const,
				Accessory const&, hana::basic_type< T > >;
		}

		/// \brief Operator for outputs
		template < typename Accessory, typename T >
		T operator()(
			std::string_view parameter_name,
			Accessory const& accessory,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< Accessory, T >()){
			return accessory.log(
				[parameter_name](logsys::stdlogb& os, T const* value){
					os << "parameter(" << parameter_name << ") ";
					if(value){
						os << "generated default value: ";
						print_if_supported(os, *value);
					}else{
						os << "no default value generated";
					}
					os << " [" << type_index::type_id< T >().pretty_name()
						<< "]";
				}, [&]()noexcept(calc_noexcept< Accessory, T >())->T{
					return std::invoke(fn_, accessory, type);
				});
		}


	private:
		Fn fn_;
	};


	struct auto_default_t{
		template < typename Accessory, typename T >
		void operator()(Accessory const&, hana::basic_type< T >)const noexcept{}

		template < typename Accessory, typename T >
		std::optional< T > operator()(
			Accessory const&,
			hana::basic_type< std::optional< T > >
		)const noexcept{ return {}; }
	};

	constexpr auto auto_default = default_value_fn(auto_default_t{});

	template < typename T >
	constexpr auto default_value(T&& value)
	noexcept(std::is_nothrow_move_constructible_v< T >){
		return default_value_fn(
			[value = std::move(value)](auto const&, auto)
			noexcept(std::is_nothrow_copy_constructible_v< T >)
			{ return value; });
	}

	constexpr auto default_value(){
		return default_value_fn([](auto const&, auto type)
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
