//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__parameter_name__hpp_INCLUDED_
#define _disposer__core__parameter_name__hpp_INCLUDED_

#include "parameter_maker.hpp"

#include "../tool/ct_name.hpp"
#include "../tool/validate_arguments.hpp"

#include <sstream>
#include <iostream>
#include <optional>
#include <tuple>


namespace disposer{


	template < char ... C >
	struct parameter_name: ct_name< C ... >{
		/// \brief Hana tag to identify parameter names
		using hana_tag = parameter_name_tag;

		/// \brief Creates a \ref parameter_maker object
		template < typename Types, typename ... Args >
		constexpr auto operator()(Types const& types, Args&& ... args)const{
			detail::validate_arguments<
					type_transform_fn_tag,
					verify_value_fn_tag,
					enable_fn_tag,
					parser_fn_tag,
					default_value_fn_tag,
					type_as_text_map_tag
				>(args ...);

			auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

			return create_parameter_maker(
				(*this),
				types,
				get_or_default(std::move(arg_tuple),
					hana::is_a< type_transform_fn_tag >,
					no_type_transform),
				get_or_default(std::move(arg_tuple),
					hana::is_a< verify_value_fn_tag >,
					verify_value_always),
				get_or_default(std::move(arg_tuple),
					hana::is_a< enable_fn_tag >,
					enable_always),
				get_or_default(std::move(arg_tuple),
					hana::is_a< parser_fn_tag >,
					stream_parser),
				get_or_default(std::move(arg_tuple),
					hana::is_a< default_value_fn_tag >,
					auto_default),
				get_or_default(std::move(arg_tuple),
					hana::is_a< type_as_text_map_tag >,
					type_as_text())
			);
		}
	};

	/// \brief Make a \ref parameter_name object
	template < char ... C >
	parameter_name< C ... > parameter_name_c{};


	/// \brief Make a \ref parameter_name object by a hana::string object
	template < char ... C > constexpr parameter_name< C ... >
	to_parameter_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_PARAMETER(s) \
	::disposer::to_parameter_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::literals{


	/// \brief Make a \ref parameter_name object via a user defined literal
	template < typename CharT, CharT ... c >
	constexpr auto operator"" _param(){
		static_assert(std::is_same_v< CharT, char >);
		return parameter_name_c< c ... >;
	}


}
#endif


#endif
