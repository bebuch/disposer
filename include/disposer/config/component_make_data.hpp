//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__component_make_data__hpp_INCLUDED_
#define _disposer__config__component_make_data__hpp_INCLUDED_

#include "embedded_config.hpp"


namespace disposer{


	/// \brief Dataset to construct and initialize a component
	struct component_make_data{
		/// \brief Name of the component from the config file
		std::string const name;

		/// \brief Name of the component type given via class
		///        declarant
		std::string const type_name;

		/// \brief Parameters from the config file
		parameter_list const parameters;

		/// \brief Header for convolved log messages
		std::string basic_location()const{
			return "component(" + name + ":" + type_name + ")";
		}

		/// \brief Header for log messages
		std::string location()const{
			return basic_location() + ": ";
		}
	};


}


#endif
