//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__print_if_supported__hpp_INCLUDED_
#define _disposer__tool__print_if_supported__hpp_INCLUDED_

#include <ostream>
#include <utility>

#include <boost/hana/is_valid.hpp>


namespace disposer{


	template < typename LogStream, typename T >
	void print_if_supported(LogStream& os, T const& v){
		auto const is_printable = hana::is_valid([](auto& x)
			->decltype((void)(std::declval< std::ostream& >() << x)){})(v);
		if constexpr(is_printable){
			os << v;
		}else{
			os << "value output on std::ostream& is not supported by type";
		}
	}


}


#endif
