//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__input_name__hpp_INCLUDED_
#define _disposer__input_name__hpp_INCLUDED_

#include "ct_name.hpp"
#include "input_maker.hpp"


namespace disposer{


	/// \brief A compile time string type for inputs
	template < char ... C >
	struct input_name: ct_name< C ... >{
		/// \brief Hana tag to identify input names
		using hana_tag = input_name_tag;

		/// \brief Creates a \ref input_maker object
		template <
			typename Types,
			typename Arg2 = no_argument,
			typename Arg3 = no_argument,
			typename Arg4 = no_argument >
		constexpr auto operator()(
			Types const& types,
			Arg2&& arg2 = {},
			Arg3&& arg3 = {},
			Arg4&& arg4 = {}
		)const{
			constexpr auto valid_argument = [](auto const& arg){
					return hana::is_a< type_transform_fn_tag >(arg)
						|| hana::is_a< verify_connection_fn_tag >(arg)
						|| hana::is_a< verify_type_fn_tag >(arg)
						|| hana::is_a< no_argument_tag >(arg);
				};

			auto const arg2_valid = valid_argument(arg2);
			static_assert(arg2_valid, "argument 2 is invalid");
			auto const arg3_valid = valid_argument(arg3);
			static_assert(arg3_valid, "argument 3 is invalid");
			auto const arg4_valid = valid_argument(arg4);
			static_assert(arg4_valid, "argument 4 is invalid");

			auto args = hana::make_tuple(
				static_cast< Arg2&& >(arg2),
				static_cast< Arg3&& >(arg3),
				static_cast< Arg4&& >(arg4)
			);

			auto tt = hana::count_if(args,
				hana::is_a< type_transform_fn_tag >) <= hana::size_c< 1 >;
			static_assert(tt, "more than one type_transform_fn");
			auto cv = hana::count_if(args,
				hana::is_a< verify_connection_fn_tag >) <= hana::size_c< 1 >;
			static_assert(cv, "more than one verify_connection_fn");
			auto tv = hana::count_if(args,
				hana::is_a< verify_type_fn_tag >) <= hana::size_c< 1 >;
			static_assert(tv, "more than one verify_type_fn");

			return create_input_maker(
				(*this),
				types,
				get_or_default(std::move(args),
					hana::is_a< type_transform_fn_tag >,
					no_type_transform),
				get_or_default(std::move(args),
					hana::is_a< verify_connection_fn_tag >,
					required),
				get_or_default(std::move(args),
					hana::is_a< verify_type_fn_tag >,
					verify_type_always)
			);
		}
	};

	/// \brief Make a \ref input_name object
	template < char ... C >
	input_name< C ... > input_name_c{};


	/// \brief Make a \ref input_name object by a hana::string object
	template < char ... C > constexpr input_name< C ... >
	to_input_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_INPUT(s) \
	::disposer::to_input_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::literals{


	/// \brief Make a \ref input_name object via a user defined literal
	template < typename CharT, CharT ... c >
	constexpr auto operator"" _in(){
		static_assert(std::is_same_v< CharT, char >);
		return input_name_c< c ... >;
	}


}
#endif


#endif
