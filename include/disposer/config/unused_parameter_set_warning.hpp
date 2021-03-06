//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__config_unused_parameter_set_warning__hpp_INCLUDED_
#define _disposer__config__config_unused_parameter_set_warning__hpp_INCLUDED_

#include "parse_config.hpp"


namespace disposer{


	/// \brief Warn about unused stuff in the parsed config via the log
	void unused_parameter_set_warning(types::parse::config const& config);


}


#endif
