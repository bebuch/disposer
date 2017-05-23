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
#include "config_fn.hpp"


namespace disposer{


	struct input_name_tag{};

	template < char ... C >
	struct input_name: ct_name< C ... >{
		using hana_tag = input_name_tag;

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
		)const;
	};

	template < char ... C >
	input_name< C ... > input_name_c{};


	template < char ... C > constexpr input_name< C ... >
	to_input_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_INPUT(s) \
	::disposer::to_input_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::literals{


	template < typename CharT, CharT ... c >
	constexpr auto operator"" _in(){
		static_assert(std::is_same_v< CharT, char >);
		return input_name_c< c ... >;
	}


}
#endif


#endif
