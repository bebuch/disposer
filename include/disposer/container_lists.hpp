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

	/// \brief Types as hana::tuple_t
	template < typename T, typename ... U >
	constexpr auto hana_type_list = hana::tuple_t< T, U ... >;

	/// \brief Map types in type_list to types in hana::tuple_t
	template < typename T, typename ... U >
	constexpr auto hana_type_list< type_list< T, U ... > > =
		hana::tuple_t< T, U ... >;


	/// \brief List of other types
	///
	/// This class is used for container inputs and outputs.
	template < typename T, typename ... U >
	struct type_list{
		/// \brief Count of types
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


	/// \brief Map the types in TypeList to the template parameters of target
	template < template< typename ... > class target, typename TypeList >
	struct type_unroll;

	/// \brief Map the types T to the template parameters of target
	template < template< typename ... > class target, typename ... T >
	struct type_unroll< target, type_list< T ... > >{
		using type = target< T ... >;
	};

	/// \brief Map the types in TypeList to the template parameters of target
	template < template< typename ... > class target, typename TypeList >
	using type_unroll_t = typename type_unroll< target, TypeList >::type;


}


#endif
