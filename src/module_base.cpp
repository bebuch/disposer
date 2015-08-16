//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/module_base.hpp>


namespace disposer{


	module_base::module_base(std::string const& type, std::string const& chain, std::string const& name):
		type(type),
		chain(chain),
		name(name)
		{}


}
