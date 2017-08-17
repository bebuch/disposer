//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_parameter__hpp_INCLUDED_
#define _disposer__core__make_parameter__hpp_INCLUDED_

#include "parameter_name.hpp"
#include "dimension.hpp"


namespace disposer{


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename DimensionConverter,
		typename ParserFn,
		typename DefaultValueFn >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter parser function
		parser_fn< ParserFn > parser;

		/// \brief Default value function
		default_value_fn< DefaultValueFn > default_value_generator;
	};


	template <,
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		typename ParserFn,
		typename DefaultValueFn >
	constexpr auto create_parameter_maker(
		parameter_name< C ... >,
		dimension_converter< Template, D ... > const&,
		parser_fn< ParserFn >&& parser,
		default_value_fn< DefaultValueFn >&& default_value_generator
	){
		return parameter_maker<
				output_name< C ... >,
				dimension_converter< Template, D ... >,
				ParserFn,
				DefaultValueFn
			>{
				std::move(parser),
				std::move(default_value_generator)
			};
	}


	/// \brief Creates a \ref parameter_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		typename ... Args >
	constexpr auto make(
		parameter_name< C ... >,
		dimension_converter< Template, D ... > const&,
		Args&& ... args
	){
		detail::validate_arguments<
			parser_fn_tag, default_value_fn_tag >(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_parameter_maker(
			parameter_name< C ... >{},
			types,
			get_or_default(std::move(arg_tuple),
				hana::is_a< parser_fn_tag >,
				stream_parser),
			get_or_default(std::move(arg_tuple),
				hana::is_a< default_value_fn_tag >,
				auto_default))
		);
	}


}


#endif
