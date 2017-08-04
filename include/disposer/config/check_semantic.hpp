//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__config_check_semantic__hpp_INCLUDED_
#define _disposer__config__config_check_semantic__hpp_INCLUDED_

#include "parse_config.hpp"


namespace disposer{


	/// \brief Check semantics of the parsed config
	///
	/// \throw std::logic_error If a semantic error is detected
	void check_semantic(types::parse::config const& config);


}


#endif
