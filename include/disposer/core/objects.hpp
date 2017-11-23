//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/objects
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__objects__hpp_INCLUDED_
#define _disposer__core__objects__hpp_INCLUDED_

#include "../tool/component_ptr.hpp"


namespace disposer{


	/// \brief Holds all makers for components and modules
	class objects{
	public:
		/// \brief Constructor
		objects() = default;


		/// \brief Not copyable
		objects(objects const&) = delete;

		/// \brief Not copyable
		objects& operator=(objects const&) = delete;


		/// \brief Get the help text of all components and modules
		std::string help()const;

		/// \brief Get the help text of a module
		std::string module_help(std::string const& name)const;

		/// \brief Get the help text of a component
		std::string component_help(std::string const& name)const;


	private:
		/// \brief List of components (map from component type name to maker
		///        function)
		component_maker_list component_maker_list_;

		/// \brief List of modules (map from module type name to maker function)
		module_maker_list module_maker_list_;
	};


}


#endif
