//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__create_chain_modules__hpp_INCLUDED_
#define _disposer__config__create_chain_modules__hpp_INCLUDED_

#include "embedded_config.hpp"

#include "../tool/module_ptr.hpp"


namespace disposer{


	/// \brief A module and its execution data
	struct chain_module_data{
		/// \brief The module
		module_ptr module;

		/// \brief The count of modules that must be ready before execution
		std::size_t precursor_count;

		/// \brief The indexes of the modules that depend on this module
		std::vector< std::size_t > next_indexes;
	};

	/// \brief List of a chains modules and indexes of the start modules
	struct chain_module_list{
		/// \brief Indexes in modules of all modules without active inputs
		std::vector< std::size_t > start_indexes;

		/// \brief List of modules and there execution data
		std::vector< chain_module_data > modules;
	};


	/// \brief Creating a chain as given by the config
	chain_module_list create_chain_modules(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain
	);


}


#endif
