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


	struct verify_connect{
		template < typename IOP_List >
		constexpr void operator()(
			IOP_List const& /*iop_list*/,
			bool /*connected*/
		)const{}
	};

	struct verify_types{
		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& /*iop_list*/,
			hana::basic_type< T > /*type*/,
			output_info const& /*info*/
		)const{}
	};


	struct input_name_tag{};

	template < char ... C >
	struct input_name: ct_name< C ... >{
		using hana_tag = input_name_tag;

		template <
			typename Types,
			typename TypesMetaFn = decltype(hana::template_< self_t >),
			typename VerifyConnectFn = verify_connection_always,
			typename VerifyTypesFn = verify_type_always >
		constexpr auto operator()(
			Types const& types,
			TypesMetaFn const& types_metaFn
				= hana::template_< self_t >,
			VerifyConnectFn&& verify_connect_fn = verify_connection_always(),
			VerifyTypesFn&& verify_type_fn = verify_type_always()
		)const noexcept;
	};

	template < char ... C >
	input_name< C ... > input_name_c{};


	template < char ... C > constexpr input_name< C ... >
	to_input_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_INPUT(s) \
	::disposer::to_input_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::interface::module{


	template < typename CharT, CharT ... c >
	constexpr auto operator"" _in(){
		static_assert(std::is_same_v< CharT, char >);
		return input_name_c< c ... >;
	}


}
#endif


#endif
