//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
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


	template < typename Container >
	std::string type_list_as_string(Container const& c){
		std::ostringstream os;
		auto iter = c.begin();
		if(iter != c.end()){
			os << '[' << *iter << ']';
			++iter;
		}
		for(; iter != c.end(); ++iter){
			os << ", [" << *iter << ']';
		}
		return os.str();
	}


}


#endif
