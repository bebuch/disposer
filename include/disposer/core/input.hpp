//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input__hpp_INCLUDED_
#define _disposer__core__input__hpp_INCLUDED_

#include "input_base.hpp"
#include "input_name.hpp"

#include <boost/hana/core/is_a.hpp>


namespace disposer{


	/// \brief Hana Tag for input
	struct input_tag{};

	/// \brief The actual input type
	template < typename Name, typename T, bool IsRequired >
	class input: public input_base{
	public:
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		static_assert(hana::is_a< input_name_tag, Name >);
#endif

		/// \brief The actual type
		using type = T;

		/// \brief Whether the input must always be connected with an output
		static constexpr auto is_required = std::bool_constant< IsRequired >();


		/// \brief Hana tag to identify inputs
		using hana_tag = input_tag;


		/// \brief Compile time name of the input
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		input(output_base* output)
			: input_base(output) {}
	};


}


#endif
