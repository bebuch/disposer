//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
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


	/// \brief Register a new module or component in directory by a name and
	///        an init function
	class declarant{
	public:
		/// \brief Not copyable
		declarant(declarant const&) = delete;

		/// \brief Not movable
		declarant(declarant&&) = delete;


		/// \brief Register a new standalone module
		///
		/// This is done by mapping the module type name to the constructing
		/// init and help function.
		void operator()(
			std::string const& type_name,
			module_maker_fn&& fn,
			std::string&& help_text);

		/// \brief Register a new component module
		///
		/// This is done by mapping the component name to a map from the module
		/// type name to the constructing init function.
		void operator()(
			std::string const& component_name,
			std::string const& module_type_name,
			component_module_maker_fn&& fn);

		/// \brief Register a new component
		///
		/// This is done by mapping the component type name to the constructing
		/// init and help function.
		void operator()(
			std::string const& type_name,
			component_maker_fn&& fn,
			std::string&& text);


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
