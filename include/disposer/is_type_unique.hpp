//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
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


	template < typename ... T >
	struct is_type_unique;

	template < typename T >
	struct is_type_unique< T >{
		static constexpr bool value = true;
	};

	template < typename T, typename U, typename ... V >
	struct is_type_unique< T, U, V ... >{
		static constexpr bool value =
			!std::is_same< T, U >::value
			&& is_type_unique< T, V ... >::value
			&& is_type_unique< U, V ... >::value;
	};

	template < typename ... T >
	constexpr bool is_type_unique_v = is_type_unique< T ... >::value;


}


#endif
