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
#include "output_info.hpp"


namespace disposer{


	struct verify_connect{
		template < typename Get >
		constexpr bool operator()(Get const& /*get*/, bool /*connected*/)const{
			return true;
		}
	};

	struct verify_all{
		template < typename Get, typename T >
		constexpr bool operator()(
			Get const& /*get*/,
			hana::basic_type< T >,
			output_info const& /*info*/
		)const{
			return true;
		}
	};


	struct input_name_tag{};

	template < char ... C >
	struct input_name: ct_name< C ... >{
		using hana_tag = input_name_tag;

		template <
			typename Types,
			typename TypesMetafunction = decltype(hana::template_< self_t >),
			typename VerifyConnectFunction = verify_connect,
			typename VerifyTypesFunction = verify_all >
		constexpr auto operator()(
			Types const& types,
			TypesMetafunction const& types_metafunction
				= hana::template_< self_t >,
			VerifyConnectFunction&& verify_connect_fn = verify_connect(),
			VerifyTypesFunction&& verify_type_fn = verify_all()
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
