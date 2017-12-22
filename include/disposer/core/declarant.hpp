//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__declarant__hpp_INCLUDED_
#define _disposer__core__declarant__hpp_INCLUDED_

#include "../tool/component_ptr.hpp"
#include "../tool/module_ptr.hpp"

#include <string>


namespace disposer{


	class directory;


	/// \brief Functor to register a new module by a name and an init function
	class declarant{
	public:
		/// \brief Not copyable
		declarant(declarant const&) = delete;

		/// \brief Not movable
		declarant(declarant&&) = delete;


		/// \brief Register a new module in the directory by mapping the
		///        module name to the constructing init and help function
		void operator()(std::string const& type_name, module_maker_fn&& fn);

		/// \brief Register a new component in the directory by mapping the
		///        component name to the constructing init and help function
		void operator()(std::string const& type_name, component_maker_fn&& fn);

		/// \brief Register the help text of a new module in the directory
		void module_help(std::string const& type_name, std::string&& text);

		/// \brief Register the help text of a new component in the directory
		void component_help(std::string const& type_name, std::string&& text);



	private:
		/// \brief Constructor
		declarant(directory& directory)
			: directory_(directory) {}


		/// \brief Reference to the directory object
		directory& directory_;


	friend class directory;
	};


}


#endif
