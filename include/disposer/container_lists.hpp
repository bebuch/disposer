//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__container_lists__hpp_INCLUDED_
#define _disposer__container_lists__hpp_INCLUDED_

#include "is_type_unique.hpp"

#include <boost/hana.hpp>

#include <functional>


namespace disposer{


	namespace hana = boost::hana;


	template < typename T, typename ... U >
	struct type_list;

	template < typename T, typename ... U >
	constexpr auto hana_type_list = hana::tuple_t< T, U ... >;

	template < typename T, typename ... U >
	constexpr auto hana_type_list< type_list< T, U ... > > =
		hana::tuple_t< T, U ... >;


	template < typename T, typename ... U >
	struct type_list{
		static constexpr std::size_t size = 1 + sizeof...(U);

		static_assert(
			!hana::fold(hana::transform(
				hana_type_list< T, U ... >,
				hana::traits::is_const
			), false, std::logical_or<>()),
			"disposer::type_list types are not allowed to be const"
		);

		static_assert(
			!hana::fold(hana::transform(
				hana_type_list< T, U ... >,
				hana::traits::is_reference
			), false, std::logical_or<>()),
			"disposer::type_list types are not allowed to be references"
		);

		static_assert(
			is_type_unique_v< T, U ... >,
			"disposer::type_list must have distict types"
		);
	};

	template < typename ... T >
	struct container_types;


	template < template< typename ... > class target, typename T >
	struct type_unroll;

	template < template< typename ... > class target, typename ... T >
	struct type_unroll< target, type_list< T ... > >{
		using type = target< T ... >;
	};

	template < template< typename ... > class target, typename T >
	using type_unroll_t = typename type_unroll< target, T >::type;


}


#endif
