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
		template <
			typename Types,
			typename Arg2 = no_argument,
			typename Arg3 = no_argument,
			typename Arg4 = no_argument,
			typename Arg5 = no_argument,
			typename Arg6 = no_argument,
			typename Arg7 = no_argument >
		constexpr auto operator()(
			Types const& types,
			Arg2&& arg2 = {},
			Arg3&& arg3 = {},
			Arg4&& arg4 = {},
			Arg5&& arg5 = {},
			Arg6&& arg6 = {},
			Arg7&& arg7 = {}
		)const{
			constexpr auto valid_argument = [](auto const& arg){
					return hana::is_a< type_transform_fn_tag >(arg)
						|| hana::is_a< verify_value_fn_tag >(arg)
						|| hana::is_a< enable_fn_tag >(arg)
						|| hana::is_a< parser_fn_tag >(arg)
						|| hana::is_a< default_value_fn_tag >(arg)
						|| hana::is_a< type_as_text_map_tag >(arg)
						|| hana::is_a< no_argument_tag >(arg);
				};

			auto const arg2_valid = valid_argument(arg2);
			static_assert(arg2_valid, "argument 2 is invalid");
			auto const arg3_valid = valid_argument(arg3);
			static_assert(arg3_valid, "argument 3 is invalid");
			auto const arg4_valid = valid_argument(arg4);
			static_assert(arg4_valid, "argument 4 is invalid");
			auto const arg5_valid = valid_argument(arg5);
			static_assert(arg5_valid, "argument 5 is invalid");
			auto const arg6_valid = valid_argument(arg6);
			static_assert(arg6_valid, "argument 6 is invalid");
			auto const arg7_valid = valid_argument(arg7);
			static_assert(arg7_valid, "argument 7 is invalid");

			auto args = hana::make_tuple(
				static_cast< Arg2&& >(arg2),
				static_cast< Arg3&& >(arg3),
				static_cast< Arg4&& >(arg4),
				static_cast< Arg5&& >(arg5),
				static_cast< Arg6&& >(arg6),
				static_cast< Arg7&& >(arg7)
			);

			auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(tt, "more than one type_transform_fn");
			auto vv = hana::count_if(args, hana::is_a< verify_value_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(vv, "more than one verify_value_fn");
			auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(ef, "more than one enable_fn");
			auto pf = hana::count_if(args, hana::is_a< parser_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(pf, "more than one parser_fn");
			auto ct = hana::count_if(args, hana::is_a< default_value_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(ct, "more than one default_value_fn");
			auto tm = hana::count_if(args, hana::is_a< type_as_text_map_tag >)
				<= hana::size_c< 1 >;
			static_assert(tm, "more than one type_as_text_map");

			return create_parameter_maker(
				(*this),
				types,
				get_or_default(std::move(args),
					hana::is_a< type_transform_fn_tag >,
					no_type_transform),
				get_or_default(std::move(args),
					hana::is_a< verify_value_fn_tag >,
					verify_value_always),
				get_or_default(std::move(args),
					hana::is_a< enable_fn_tag >,
					enable_always),
				get_or_default(std::move(args),
					hana::is_a< parser_fn_tag >,
					stream_parser),
				get_or_default(std::move(args),
					hana::is_a< default_value_fn_tag >,
					auto_default),
				get_or_default(std::move(args),
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
