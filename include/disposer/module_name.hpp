//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_name__hpp_INCLUDED_
#define _disposer__module_name__hpp_INCLUDED_

#include "ct_name.hpp"


namespace disposer{


	struct module_name_tag{};

	template < char ... C >
	struct module_name: ct_name< C ... >{
		using hana_tag = module_name_tag;

		template < typename ... IO >
		constexpr auto operator()(io< IO > const& ... ios)const noexcept;
	};

	template < char ... C >
	module_name< C ... > module_name_c{};


	template < char ... C > constexpr module_name< C ... >
	to_module_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_MODULE(s) \
	::disposer::to_module_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::interface::module{


	template < typename CharT, CharT ... c >
	constexpr auto operator"" _module(){
		static_assert(std::is_same_v< CharT, char >);
		return module_name_c< c ... >;
	}


}
#endif


#endif
