//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__make_map_with__hpp_INCLUDED_
#define _disposer__make_map_with__hpp_INCLUDED_

#include <boost/hana.hpp>


namespace disposer{


	namespace hana = boost::hana;


	template < typename Types, typename Value >
	constexpr auto make_map_with(
		hana::set< Types > set,
		Value const& value
	){
		return hana::unpack(set, hana::make_map ^ hana::on ^
			hana::curry< 2 >(hana::flip(hana::make_pair))(value));
	}

	template < typename Types, typename Value >
	constexpr auto make_map_with(
		Value const& value,
		hana::set< Types > set
	){
		return hana::unpack(set, hana::make_map ^ hana::on ^
			hana::curry< 2 >(hana::make_pair)(value));
	}


}


#endif
