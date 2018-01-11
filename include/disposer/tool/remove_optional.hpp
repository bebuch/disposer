//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__remove_optional__hpp_INCLUDED_
#define _disposer__tool__remove_optional__hpp_INCLUDED_

#include <boost/hana/type.hpp>

#include <type_traits>
#include <optional>


namespace disposer{


	namespace hana = boost::hana;


}


namespace disposer::detail{


	template < typename T >
	struct is_optional: std::false_type{};

	template < typename T >
	struct is_optional< std::optional< T > >: std::true_type{};

	template < typename T >
	struct is_optional< T& >: is_optional< T >{};

	template < typename T >
	struct is_optional< T&& >: is_optional< T >{};

	template < typename T >
	struct is_optional< T const >: is_optional< T >{};

	template < typename T >
	struct is_optional< T volatile >: is_optional< T >{};

	template < typename T >
	struct is_optional< T const volatile >: is_optional< T >{};

	template < typename T >
	constexpr bool is_optional_v = is_optional< T >::value;


	struct remove_optional_t{
		template < typename T >
		constexpr auto operator()(hana::basic_type< T >)const noexcept{
			if constexpr(is_optional_v< T >){
				return hana::type_c< typename T::value_type >;
			}else{
				return hana::type_c< T >;
			}
		}
	};

	constexpr auto remove_optional = remove_optional_t{};


}


#endif
