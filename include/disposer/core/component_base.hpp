//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component_base__hpp_INCLUDED_
#define _disposer__core__component_base__hpp_INCLUDED_

#include "disposer.hpp"


namespace disposer{


	/// \brief Base class for all disposer components
	class component_base{
	public:
		/// \brief Constructor
		component_base(
			std::string const& name,
			std::string const& type_name
		)
			: name(name)
			, type_name(type_name)
			, location("component(" + name + ":" + type_name + "): ") {}


		/// \brief Components are not copyable
		component_base(component_base const&) = delete;

		/// \brief Components are not movable
		component_base(component_base&&) = delete;


		/// \brief Components are not copyable
		component_base& operator=(component_base const&) = delete;

		/// \brief Components are not movable
		component_base& operator=(component_base&&) = delete;


		/// \brief Standard virtual destructor
		virtual ~component_base() = default;


		/// \brief shutdown component if it has a shutdown function
		virtual void shutdown() = 0;


		/// \brief Name of the component in config file section 'component'
		std::string const name;

		/// \brief Name of the component type given via class
		///        component_declarant
		std::string const type_name;

		/// \brief Location text for log-messages
		std::string const location;
	};


}


#endif
