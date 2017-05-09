//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__parameter_name__hpp_INCLUDED_
#define _disposer__parameter_name__hpp_INCLUDED_

#include "ct_name.hpp"

#include <sstream>
#include <iostream>
#include <optional>
#include <tuple>


namespace disposer{


	struct parameter_parser{
		template < typename T >
		T operator()(std::string_view value, hana::basic_type< T >)const{
			if constexpr(std::is_same_v< T, std::string >){
				return std::string(value);
			}else{
				std::istringstream is((std::string(value)));
				T result;
				if constexpr(std::is_same_v< T, bool >){
					is >> std::boolalpha;
				}
				is >> result;
				return result;
			}
		}
	};

	template < typename Types >
	struct default_value_type_impl{
		using typelist = std::conditional_t<
			hana::is_a< hana::type_tag, Types >,
			hana::tuple< Types >,
			Types >;

		using type = std::optional< typename decltype(hana::unpack(
			typelist{}, hana::template_< std::tuple >))::type >;
	};

	template < typename Types >
	using default_value_type =
		typename default_value_type_impl< Types >::type;


	struct parameter_name_tag{};

	template < char ... C >
	struct parameter_name: ct_name< C ... >{
		using hana_tag = parameter_name_tag;

		template <
			typename Types,
			typename EnableFunction = enable_all,
			typename ParserFunction = parameter_parser,
			typename AsText = hana::map<> >
		constexpr auto operator()(
			Types const& types,
			EnableFunction&& enable_fn = enable_all(),
			ParserFunction&& parser_fn = parameter_parser(),
			default_value_type< Types >&& default_values = {},
			AsText&& to_text = hana::make_map()
		)const noexcept;
	};

	template < char ... C >
	parameter_name< C ... > parameter_name_c{};


	template < char ... C > constexpr parameter_name< C ... >
	to_parameter_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_PARAMETER(s) \
	::disposer::to_parameter_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::interface::module{


	template < typename CharT, CharT ... c >
	constexpr auto operator"" _param(){
		static_assert(std::is_same_v< CharT, char >);
		return parameter_name_c< c ... >;
	}


}
#endif


#endif
