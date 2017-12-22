//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__disposer_declarant__hpp_INCLUDED_
#define _disposer__core__disposer_declarant__hpp_INCLUDED_

#include "../tool/component_ptr.hpp"
#include "../tool/module_ptr.hpp"

#include <string>


namespace disposer{


	class system;


	/// \brief Functor to register a new module by a name and an init function
	class module_declarant{
	public:
		/// \brief Not copyable
		module_declarant(module_declarant const&) = delete;

		/// \brief Not movable
		module_declarant(module_declarant&&) = delete;


		/// \brief Register a new module in the system by mapping the
		///        module name to the constructing init and help function
		void operator()(std::string const& type_name, module_maker_fn&& fn);

		/// \brief Register the help text of a new module in the system
		void help(std::string const& type_name, std::string&& text);


	private:
		/// \brief Constructor
		module_declarant(system& system): system_(system) {}


		/// \brief Reference to the system object
		system& system_;


	friend class system;
	};


	/// \brief Functor to register a new component by a name and an init
	///        function
	class component_declarant{
	public:
		/// \brief Not copyable
		component_declarant(component_declarant const&) = delete;

		/// \brief Not movable
		component_declarant(component_declarant&&) = delete;


		/// \brief Register a new component in the system by mapping the
		///        component name to the constructing init and help function
		void operator()(std::string const& type_name, component_maker_fn&& fn);

		/// \brief Register the help text of a new component in the system
		void help(std::string const& type_name, std::string&& text);

		/// \brief Get a reference to the system object
		disposer::system& system(){ return system_; }


	private:
		/// \brief Constructor
		component_declarant(disposer::system& system)
			: system_(system) {}


		/// \brief Reference to the system object
		disposer::system& system_;


	friend class system;
	};


}


#endif
