//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__io__hpp_INCLUDED_
#define _disposer__io__hpp_INCLUDED_

#include <boost/hana.hpp>


namespace disposer{


	namespace hana = ::boost::hana;


}


namespace disposer::interface::module{


	template < typename Name, typename Types >
	struct in;

	template < typename T >
	struct is_an_in: std::false_type{};

	template < typename Name, typename Types >
	struct is_an_in< in< Name, Types > >: std::true_type{};

	template < typename T >
	constexpr bool is_an_in_v = is_an_in< T >::value;


	template < typename Name, typename Types >
	struct out;

	template < typename T >
	struct is_an_out: std::false_type{};

	template < typename Name, typename Types >
	struct is_an_out< out< Name, Types > >: std::true_type{};

	template < typename T >
	constexpr bool is_an_out_v = is_an_out< T >::value;


	/// \brief Base of input and output type deduction classes
	template < typename IO >
	struct io{};


	/// \brief Create two boost::hana::map's from the given input and output
	///        types
	template < typename ... IO >
	constexpr auto io_list(io< IO > ...){
		constexpr auto types = hana::tuple_t< IO ... >;
		constexpr auto inputs = hana::filter(types, [](auto&& type){
				return hana::bool_c< is_an_in_v< typename decltype(+type)::type > >;
			});
		constexpr auto outputs = hana::filter(types, [](auto&& type){
				return hana::bool_c< is_an_out_v< typename decltype(+type)::type > >;
			});

		constexpr auto make_list = [](auto ... io){
			return hana::type_c< hana::map< hana::pair<
				typename decltype(+io)::type::name,
				typename decltype(+io)::type::type > ... > >;
		};

		return hana::make_pair(
			hana::unpack(inputs, make_list), hana::unpack(outputs, make_list));
	}


}


#endif
