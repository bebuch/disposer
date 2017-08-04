//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__false_c__hpp_INCLUDED_
#define _disposer__tool__false_c__hpp_INCLUDED_


namespace disposer::detail{


	/// \brief Utility to make the value false template parameter dependet
	template < typename >
	constexpr bool false_c = false;


}


#endif
