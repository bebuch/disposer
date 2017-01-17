//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__id_generator__hpp_INCLUDED_
#define _disposer__id_generator__hpp_INCLUDED_

#include <atomic>


namespace disposer{


	struct id_generator{
		id_generator(): next_id(0){}
		id_generator(id_generator const&) = delete;
		id_generator(id_generator&&) = delete;

		id_generator& operator=(id_generator const&) = delete;
		id_generator& operator=(id_generator&&) = delete;

		std::size_t operator()(std::size_t increase){
			return next_id.fetch_add(increase);
		}

		std::atomic< std::size_t > next_id;
	};


}


#endif
