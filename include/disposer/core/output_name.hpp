//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_name__hpp_INCLUDED_
#define _disposer__core__output_name__hpp_INCLUDED_

#include "../tool/ct_name.hpp"


namespace disposer{


	/// \brief Hana Tag for output_maker
	struct output_maker_tag{};

	/// \brief Hana Tag for \ref output_name
	struct output_name_tag{};

	/// \brief A compile time string type for outputs
	template < char ... C >
	struct output_name: ct_name< output_name_tag, C ... >{};

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
