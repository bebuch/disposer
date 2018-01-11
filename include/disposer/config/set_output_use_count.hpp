//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__set_output_use_count__hpp_INCLUDED_
#define _disposer__config__set_output_use_count__hpp_INCLUDED_

#include "embedded_config.hpp"


namespace disposer{


	/// \brief Set output use_count to the count of connected inputs
	void set_output_use_count(types::embedded_config::config& config);

	/// \brief Set output use_count to the count of connected inputs
	void set_output_use_count(types::embedded_config::chain& config);


}


#endif
