//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__is_type_unique__hpp_INCLUDED_
#define _disposer__is_type_unique__hpp_INCLUDED_

#include <type_traits>


namespace disposer{


	/// \brief true if all types T are distinct, false otherwise
	template < typename ... T >
	struct is_type_unique;

	/// \brief true, because T is the only type
	template < typename T >
	struct is_type_unique< T >{
		static constexpr bool value = true;
	};

	/// \brief true if all types T, U, V... are distinct, false otherwise
	template < typename T, typename U, typename ... V >
	struct is_type_unique< T, U, V ... >{
		static constexpr bool value =
			!std::is_same< T, U >::value
			&& is_type_unique< T, V ... >::value
			&& is_type_unique< U, V ... >::value;
	};

	/// \brief true if all types T are distinct, false otherwise
	template < typename ... T >
	constexpr bool is_type_unique_v = is_type_unique< T ... >::value;


}


#endif
