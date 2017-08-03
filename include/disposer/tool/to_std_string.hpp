//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__to_std_string__hpp_INCLUDED_
#define _disposer__tool__to_std_string__hpp_INCLUDED_

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/string.hpp>
#include <boost/hana/size.hpp>

#include <string>


namespace disposer{


	namespace hana = boost::hana;


}


namespace disposer::detail{


	/// \brief Converts a hana::string to std::string
	template < typename String,
		typename = std::enable_if< hana::is_a< hana::string_tag, String >() > >
	std::string to_std_string(String const& s){
		return {s.c_str(), hana::size(s)};
	}


}


#endif
