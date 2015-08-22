//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_make_data_hpp_INCLUDED_
#define _disposer_make_data_hpp_INCLUDED_

#include "parameter_processor.hpp"

#include <unordered_set>


namespace disposer{


	using io_list = std::unordered_set< std::string >;

	struct make_data{
		std::string const type_name;
		std::string const chain;
		std::string const name;
		std::size_t const number;
		io_list const inputs;
		io_list const outputs;
		parameter_processor params;

		bool is_first()const{ return number == 0; }
	};


}


#endif
