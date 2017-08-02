//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_make_data__hpp_INCLUDED_
#define _disposer__module_make_data__hpp_INCLUDED_

#include "embedded_config.hpp"


namespace disposer{


	class output_base;


	/// \brief Dataset for disposer to construct and initialize a module
	struct module_make_data{
		/// \brief Name of the module type given via class module_declarant
		std::string const type_name;

		/// \brief Name of the process chain in config file section 'chain'
		std::string const chain;

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 1.
		std::size_t const number;

		/// \brief Map of the input names and corresponding output pointers
		input_list const inputs;

		/// \brief List of the output names
		output_list const outputs;

		/// \brief Parameters from the config file
		parameter_list const parameters;

		/// \brief Header for convolved log messages
		std::string basic_location()const{
			return "chain(" + chain + ") module("
				+ std::to_string(number) + ":" + type_name + ")";
		}

		/// \brief Header for log messages
		std::string location()const{
			return basic_location() + ": ";
		}
	};


}


#endif
