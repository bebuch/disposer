//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input_name__hpp_INCLUDED_
#define _disposer__core__input_name__hpp_INCLUDED_

#include "../tool/ct_name.hpp"


namespace disposer{


	/// \brief Hana Tag for input_maker
	struct input_maker_tag{};

	/// \brief Hana Tag for \ref input_name
	struct input_name_tag{};

	/// \brief A compile time string type for inputs
	template < char ... C >
	struct input_name: ct_name< input_name_tag, C ... >{};

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
