//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_container_lists_hpp_INCLUDED_
#define _disposer_container_lists_hpp_INCLUDED_

#include "is_type_unique.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/type_traits.hpp>


namespace disposer{


	namespace hana = boost::hana;

	template < typename T, typename ... U >
	struct type_list{
		static constexpr auto hana_list = hana::tuple_t< T, U ... >;

		static constexpr std::size_t size = 1 + sizeof...(U);

		static_assert(
			!hana::fold(hana::transform(hana_list, hana::traits::is_const), false, std::logical_or<>()),
			"disposer::type_list types are not allowed to be const"
		);

		static_assert(
			!hana::fold(hana::transform(hana_list, hana::traits::is_reference), false, std::logical_or<>()),
			"disposer::type_list types are not allowed to be references"
		);

		static_assert(is_type_unique< T, U ... >, "disposer::type_list must have distict types");
	};

	template < typename ... T >
	struct container_types;


}


#endif
