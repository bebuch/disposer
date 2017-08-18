//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__comma_separated_output__hpp_INCLUDED_
#define _disposer__tool__comma_separated_output__hpp_INCLUDED_

#include <tuple>
#include <type_traits>


namespace disposer::detail{


	template < typename T >
	is_tuple: std::false_type{};

	template < typename ... T >
	is_tuple< std::tuple< T ... > >: std::true_type{};

	template < typename T >
	constexpr bool is_tuple_v = is_tuple< T >::value;


	template < typename OStream, typename Arg, typename ... Args >
	void comma_separated_output(
		OStream& os,
		Arg const& arg,
		Args const& ... args
	){
		if constexpr(is_tuple_v< Arg >){
			os << arg;
			(os << ", " << ... << args);
		}else{
			constexpr auto print = [&os](auto const& ... args){
				return (os << ... << args);
			};
			std::apply(print, arg);
			(os << ", " << ... << std::apply(print, args));
		}
	}


}


#endif
