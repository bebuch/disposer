//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__unpack_to__hpp_INCLUDED_
#define _disposer__unpack_to__hpp_INCLUDED_

#include <boost/hana.hpp>


namespace disposer{


	namespace hana = boost::hana;


	template < template < typename ... > typename Class >
	struct unpack_to_t{
		template < typename Seq >
		auto operator()(Seq&& seq)const{
			return hana::unpack(static_cast< Seq&& >(seq), [](auto&& ... types){
					return hana::type_c< Class<
						typename decltype(+types)::type ... > >;
				});
		}
	};

	template < template < typename ... > typename Class >
	constexpr unpack_to_t< Class > unpack_to{};


}


#endif
