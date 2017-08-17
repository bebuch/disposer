//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_input__hpp_INCLUDED_
#define _disposer__core__make_input__hpp_INCLUDED_

#include "input_name.hpp"
#include "dimension.hpp"


namespace disposer{


	/// \brief Provids types for constructing an input
	template <
		typename Name,
		typename DimensionConverter,
		bool IsRequired >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;
	};


	/// \brief Configuration class for inputs
	template < bool IsRequired >
	struct is_required: std::bool_constant< IsRequired >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto required = is_required< true >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto not_required = is_required< false >{};


	/// \brief Creates a \ref input_maker object
	template <
		char ... C
		template < typename ... > typename Template,
		std::size_t ... D,
		bool IsRequired = true >
	constexpr auto make(
		input_name< C ... > const&,
		dimension_converter< Template, D ... > const&,
		is_required< IsRequired > = required
	){
		return input_maker<
			input_name< C ... >,
			dimension_converter< Template, D ... >,
			IsRequired >{};
	}



}


#endif
