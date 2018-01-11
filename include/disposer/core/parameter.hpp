//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__parameter__hpp_INCLUDED_
#define _disposer__core__parameter__hpp_INCLUDED_

#include "parameter_name.hpp"


namespace disposer{


	/// \brief Hana Tag for parameter
	struct parameter_tag{};

	/// \brief The parameter type
	template < typename Name, typename T >
	class parameter{
	public:
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		static_assert(hana::is_a< parameter_name_tag, Name >);
#endif

		/// \brief Hana tag to identify parameters
		using hana_tag = parameter_tag;


		/// \brief Compile time name of the parameter
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		template < typename ... Args >
		parameter(Args&& ... args):
			value_(static_cast< Args&& >(args) ...){}

		/// \brief Get reference to value
		T const& get()const{
			return value_;
		}


	private:
		/// \brief The value
		T const value_;
	};


}


#endif
