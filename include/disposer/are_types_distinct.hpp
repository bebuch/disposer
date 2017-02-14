//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__are_types_distinct__hpp_INCLUDED_
#define _disposer__are_types_distinct__hpp_INCLUDED_

#include <type_traits>


namespace disposer{


	/// \brief true if all types T are distinct, false otherwise
	template < typename ... T >
	struct are_types_distinct;

	/// \brief true, because T is the only type
	template < typename T >
	struct are_types_distinct< T >{
		static constexpr bool value = true;
	};

	/// \brief true if all types T, U, V... are distinct, false otherwise
	template < typename T, typename U, typename ... V >
	struct are_types_distinct< T, U, V ... >{
		static constexpr bool value =
			!std::is_same< T, U >::value
			&& are_types_distinct< T, V ... >::value
			&& are_types_distinct< U, V ... >::value;
	};

	/// \brief true if all types T are distinct, false otherwise
	template < typename ... T >
	constexpr bool are_types_distinct_v = are_types_distinct< T ... >::value;


}


#endif
