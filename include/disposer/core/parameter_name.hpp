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

#include "../tool/ct_name.hpp"


namespace disposer{


	/// \brief Hana Tag for parameter_maker
	struct parameter_maker_tag{};

	/// \brief Hana Tag for \ref parameter_name
	struct parameter_name_tag{};

	/// \brief A compile time string type for parameters
	template < char ... C >
	struct parameter_name: ct_name< C ... >{
		/// \brief Hana tag to identify parameter names
		using hana_tag = parameter_name_tag;
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
