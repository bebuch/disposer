//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_ptr__hpp_INCLUDED_
#define _disposer__module_ptr__hpp_INCLUDED_

#include <functional>
#include <memory>


namespace disposer{


	class module_base;

	/// \brief unique_ptr to class module_base or derived classes
	using module_ptr = std::unique_ptr< module_base >;


}


#endif
