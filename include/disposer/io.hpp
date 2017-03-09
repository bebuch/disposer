//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__io__hpp_INCLUDED_
#define _disposer__io__hpp_INCLUDED_

#include <boost/hana.hpp>


namespace disposer{


	namespace hana = ::boost::hana;


}


namespace disposer::interface::module{


	/// \brief Base of input and output type deduction classes
	template < typename IO >
	struct io{};


	/// \brief Create two boost::hana::map's from the given input and output
	///        types
	template < typename ... IO >
	constexpr auto io_list(io< IO > ...){
		return hana::map< hana::pair<
			typename IO::name, typename IO::type > ... >{};
	}


}


#endif
