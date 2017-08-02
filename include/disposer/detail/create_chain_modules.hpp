//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__create_chain_modules__hpp_INCLUDED_
#define _disposer__create_chain_modules__hpp_INCLUDED_

#include "module_ptr.hpp"
#include "embedded_config.hpp"


namespace disposer{


	/// \brief Creating a chain as given by the config
	std::vector< module_ptr > create_chain_modules(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain
	);


}


#endif
