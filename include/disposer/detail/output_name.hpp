//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__output_name__hpp_INCLUDED_
#define _disposer__output_name__hpp_INCLUDED_

#include "output_maker.hpp"

#include "../tool/ct_name.hpp"


namespace disposer{


	/// \brief A compile time string type for outputs
	template < char ... C >
	struct output_name: ct_name< C ... >{
		/// \brief Hana tag to identify output names
		using hana_tag = output_name_tag;

		/// \brief Creates a \ref output_maker object
		template <
			typename Types,
			typename Arg2 = no_argument,
			typename Arg3 = no_argument >
		constexpr auto operator()(
			Types const& types,
			Arg2&& arg2 = {},
			Arg3&& arg3 = {}
		)const{
			constexpr auto valid_argument = [](auto const& arg){
					return hana::is_a< type_transform_fn_tag >(arg)
						|| hana::is_a< enable_fn_tag >(arg)
						|| hana::is_a< no_argument_tag >(arg);
				};

			auto const arg2_valid = valid_argument(arg2);
			static_assert(arg2_valid, "argument 2 is invalid");
			auto const arg3_valid = valid_argument(arg3);
			static_assert(arg3_valid, "argument 3 is invalid");

			auto args = hana::make_tuple(
				static_cast< Arg2&& >(arg2),
				static_cast< Arg3&& >(arg3)
			);

			auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(tt, "more than one type_transform_fn");
			auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
				<= hana::size_c< 1 >;
			static_assert(ef, "more than one enable_fn");

			return create_output_maker(
				(*this),
				types,
				get_or_default(std::move(args),
					hana::is_a< type_transform_fn_tag >,
					no_type_transform),
				get_or_default(std::move(args),
					hana::is_a< enable_fn_tag >,
					enable_always)
			);
		}
	};

	/// \brief Make a \ref output_name object
	template < char ... C >
	output_name< C ... > output_name_c{};


	/// \brief Make a \ref output_name object by a hana::string object
	template < char ... C > constexpr output_name< C ... >
	to_output_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_OUTPUT(s) \
	::disposer::to_output_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::literals{


	/// \brief Make a \ref output_name object via a user defined literal
	template < typename CharT, CharT ... c >
	constexpr auto operator"" _out(){
		static_assert(std::is_same_v< CharT, char >);
		return output_name_c< c ... >;
	}


}
#endif


#endif
