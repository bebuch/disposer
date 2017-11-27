//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__comma_separated_output__hpp_INCLUDED_
#define _disposer__tool__comma_separated_output__hpp_INCLUDED_

#include "ct_pretty_name.hpp"

#include <tuple>
#include <type_traits>

#include <boost/hana/size.hpp>
#include <boost/hana/range.hpp>


namespace disposer::detail{


	template < typename T >
	struct is_tuple: std::false_type{};

	template < typename ... T >
	struct is_tuple< std::tuple< T ... > >: std::true_type{};

	template < typename T >
	constexpr bool is_tuple_v = is_tuple< T >::value;


	template < typename OStream, typename Arg, typename ... Args >
	void comma_separated_output(
		OStream& os,
		Arg const& arg,
		Args const& ... args
	){
		if constexpr(!is_tuple_v< Arg >){
			os << arg;
			([&os](auto const& arg){ os << ", " << arg; }(args), ...);
		}else{
			std::apply([&os](auto const& ... args)mutable{
					(os << ... << args);
				}, arg);
			(std::apply([&os](auto const& ... args)mutable{
					os << ", ";
					(os << ... << args);
				}, args), ...);
		}
	}


	template < typename Types >
	std::string get_type_name(std::size_t i, Types types){
		using type_count = decltype(hana::size(types));

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		assert(i < type_count::value);
#endif

		return hana::unpack(
			hana::make_range(hana::size_c< 0 >, hana::size(types)),
			[i](auto ... I){
				constexpr auto name_fn = [](auto I){
						return []{
								return ct_pretty_name< typename
										decltype(+types[I])::type
									>();
							};
					};

				using fn_type = std::string(*)();
				constexpr fn_type names[type_count::value]
					= {static_cast< fn_type >(name_fn(decltype(I){})) ...};
				return names[i]();
			});
	}


}


#endif
