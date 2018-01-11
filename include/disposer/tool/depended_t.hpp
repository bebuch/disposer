//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__depended_t__hpp_INCLUDED_
#define _disposer__tool__depended_t__hpp_INCLUDED_


namespace disposer::detail{


	/// \brief Utility to get the same number of type as one or more variadic
	///        template parameters
	///
	/// Example:
	///
	///    template < typename ... T >
	///    std::tuple< depended_t< int, T > ... > f(T&& v){ /* ... */ }
	template < typename T, typename, typename ... >
	using depended_t = T;

	/// \brief Same as depended_t but with value template parameters
	template < typename T, auto, auto ... >
	using value_depended_t = T;


}


#endif
