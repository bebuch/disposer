//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_map_type__hpp_INCLUDED_
#define _disposer__core__output_map_type__hpp_INCLUDED_

#include "output_base.hpp"
#include "exec_output_base.hpp"

#include <unordered_map>


namespace disposer{


	using output_map_type
		= std::unordered_map< output_base*, exec_output_base* >;


}


#endif
