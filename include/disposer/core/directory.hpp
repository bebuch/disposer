//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__directory__hpp_INCLUDED_
#define _disposer__core__directory__hpp_INCLUDED_

#include "../tool/component_ptr.hpp"
#include "../tool/module_ptr.hpp"

#include <vector>
#include <map>


namespace disposer{


	class module_declarant;
	class component_declarant;


	/// \brief Component and module makers
	class directory{
	public:
		/// \brief Constructor
		directory() = default;


		/// \brief Not copyable
		directory(directory const&) = delete;

		/// \brief Not movable
		directory(directory&&) = delete;


		/// \brief Not copyable
		directory& operator=(directory const&) = delete;

		/// \brief Not movable
		directory& operator=(directory&&) = delete;


		/// \brief Get the help text of all components and modules
		std::string help()const;

		/// \brief Get the help text of a module
		std::string module_help(std::string const& name)const;

		/// \brief Get the help text of a component
		std::string component_help(std::string const& name)const;


		/// \brief Get all module names
		std::vector< std::string > module_names()const;

		/// \brief Get all component names
		std::vector< std::string > component_names()const;


	private:
		/// \brief Map from component type name to help text
		std::map< std::string, std::string > component_help_list_;

		/// \brief Map from module type name to help text
		std::map< std::string, std::string > module_help_list_;


		/// \brief List of components (map from component type name to maker
		///        function)
		component_maker_list component_maker_list_;

		/// \brief List of modules (map from module type name to maker function)
		module_maker_list module_maker_list_;


	friend class system;
	friend class module_declarant;
	friend class component_declarant;
	};


}


#endif
