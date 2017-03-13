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

#include "ct_name.hpp"
#include "io.hpp"


namespace disposer{


	struct output_name_tag{};

	template < char ... C >
	struct output_name: ct_name< C ... >{
		using hana_tag = output_name_tag;

		template < typename Types >
		constexpr auto operator()(Types&& types)const noexcept;
	};

	template < char ... C >
	output_name< C ... > output_name_c{};


	template < char ... C > constexpr output_name< C ... >
	to_output_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_OUTPUT_NAME(s) \
	::disposer::to_output_name(BOOST_HANA_STROUTG(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::interface::module{


	template < typename CharT, CharT ... c >
	constexpr auto operator"" _out(){
		static_assert(std::is_same_v< CharT, char >);
		return output_name_c< c ... >;
	}


}
#endif


#endif
