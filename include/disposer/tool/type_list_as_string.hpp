//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__type_list_as_string__hpp_INCLUDED_
#define _disposer__tool__type_list_as_string__hpp_INCLUDED_

#include "type_index.hpp"

#include <sstream>


namespace disposer{


	inline std::string type_list_as_string(
		std::unordered_set< type_index > const& list
	){
		std::ostringstream os;
		auto iter = begin(list);
		auto const e = end(list);
		if(iter != e){
			os << '[' << iter->pretty_name() << ']';
			++iter;
		}
		for(; iter != e; ++iter){
			os << ", [" << iter->pretty_name() << ']';
		}
		return os.str();
	}


}


#endif
