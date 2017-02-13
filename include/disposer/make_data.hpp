//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__make_data__hpp_INCLUDED_
#define _disposer__make_data__hpp_INCLUDED_

#include "parameter_processor.hpp"

#include <unordered_set>


namespace disposer{


	/// \brief Type for input and output name lists
	using io_list = std::unordered_set< std::string >;


	/// \brief Dataset for disposer to construct and initialize a module
	struct make_data{
		/// \brief Name of the module type given via class module_adder
		std::string const type_name;

		/// \brief Name of the process chain in config file section 'chain'
		std::string const chain;

		/// \brief Name of the module in config file section 'module'
		std::string const name;

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 0, the second 1 and so on.
		std::size_t const number;

		/// \brief List of the input names
		io_list const inputs;

		/// \brief List of the output names
		io_list const outputs;

		/// \brief Parameters from the config file
		parameter_processor params;


		/// \brief true if module is the first in the chain, false otherwise
		bool is_first()const{ return number == 0; }
	};


}


#endif
