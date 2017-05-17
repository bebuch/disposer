//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__ct_name__hpp_INCLUDED_
#define _disposer__ct_name__hpp_INCLUDED_

#include <boost/hana.hpp>

#include <array>


namespace disposer{


	namespace hana = boost::hana;


	template < std::size_t N >
	constexpr bool valid_name(std::array< char, N > const& name){
		for(auto c: name){
			if(!(
				(c >= 'A' && c <= 'U') ||
				(c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') ||
				c == '_'
			)) return false;
		}
		return true;
	}


	template < char ... C >
	struct ct_name{
		static_assert(valid_name(std::array< char, sizeof...(C) >{{ C ... }}),
			"only [A-Za-z0-9_] are valid characters");

		static constexpr auto value = hana::string_c< C ... >;

		using value_type = std::remove_cv_t< decltype(value) >;
	};


}


#endif
