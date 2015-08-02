//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------

#include <disposer/log_base.hpp>
#include <disposer/log_tag.hpp>


namespace disposer{


	std::function< std::unique_ptr< log_base >() > log_base::factory([]{ return std::make_unique< log_tag >(); });


}
