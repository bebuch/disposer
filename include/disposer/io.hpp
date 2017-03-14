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


	struct in_tag{};
	struct out_tag{};

	/// \brief Base of input and output type deduction classes
	template < typename IO >
	struct io{};


	/// \brief Create two boost::hana::map's from the given input and output
	///        types
	template < typename ... IO >
	constexpr auto make_io_lists(io< IO > ...){
		constexpr auto types = hana::tuple_t< IO ... >;
		constexpr auto inputs = hana::filter(types, [](auto&& type){
				return hana::is_a< in_tag, typename decltype(+type)::type >;
			});
		constexpr auto outputs = hana::filter(types, [](auto&& type){
				return hana::is_a< out_tag, typename decltype(+type)::type >;
			});

		constexpr auto io_type_list = [](auto ... io){
			return hana::type_c< hana::map< hana::pair<
				typename decltype(+io)::type::name::value_type,
				typename decltype(+io)::type::type > ... > >;
		};

		return hana::make_pair(
			hana::unpack(inputs, io_type_list),
			hana::unpack(outputs, io_type_list));
	}


}


#endif
