//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_output__hpp_INCLUDED_
#define _disposer__core__make_output__hpp_INCLUDED_

#include "input_name.hpp"
#include "dimension.hpp"


namespace disposer{


	/// \brief Provid types for constructing an output
	template <
		typename Name,
		typename DimensionConverter >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;
	};


	/// \brief Creates a \ref output_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D >
	constexpr auto make(
		output_name< C ... > const&,
		dimension_converter< Template, D ... > const&
	){
		return output_maker<
			output_name< C ... >,
			dimension_converter< Template, D ... > >{};
	}


}


#endif
