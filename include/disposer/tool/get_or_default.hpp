//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__get_or_default__hpp_INCLUDED_
#define _disposer__tool__get_or_default__hpp_INCLUDED_

#include <boost/hana/count_if.hpp>
#include <boost/hana/size.hpp>


namespace disposer{


	template < typename Tuple, typename Predicate, typename Default >
	constexpr auto get_or_default(
		Tuple&& tuple,
		Predicate&& predicate,
		Default&& default_value
	){
		auto const count = hana::count_if(tuple, predicate);
		static_assert(count <= hana::size_c< 1 >,
			"more than one argument with this tag");

		auto result = hana::find_if(static_cast< Tuple&& >(tuple),
			static_cast< Predicate&& >(predicate));
		if constexpr(auto const found = result != hana::nothing; found){
			return *static_cast< decltype(result)&& >(result);
		}else{
			return static_cast< Default&& >(default_value);
		}
	}


}


#endif
