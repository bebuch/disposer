//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__as_text__hpp_INCLUDED_
#define _disposer__as_text__hpp_INCLUDED_

#include <boost/hana.hpp>

#include <cstdint>


namespace disposer{


	namespace hana = boost::hana;

	constexpr auto as_text = hana::make_map(
		hana::make_pair(hana::type_c< bool >, BOOST_HANA_STRING("bool")),
		hana::make_pair(hana::type_c< char >, BOOST_HANA_STRING("char")),
		hana::make_pair(hana::type_c< std::int8_t >,
			BOOST_HANA_STRING("sint8")),
		hana::make_pair(hana::type_c< std::uint8_t >,
			BOOST_HANA_STRING("uint8")),
		hana::make_pair(hana::type_c< std::int16_t >,
			BOOST_HANA_STRING("sint16")),
		hana::make_pair(hana::type_c< std::uint16_t >,
			BOOST_HANA_STRING("uint16")),
		hana::make_pair(hana::type_c< std::int32_t >,
			BOOST_HANA_STRING("sint32")),
		hana::make_pair(hana::type_c< std::uint32_t >,
			BOOST_HANA_STRING("uint32")),
		hana::make_pair(hana::type_c< std::int64_t >,
			BOOST_HANA_STRING("sint64")),
		hana::make_pair(hana::type_c< std::uint64_t >,
			BOOST_HANA_STRING("uint64")),
		hana::make_pair(hana::type_c< float >,  BOOST_HANA_STRING("float32")),
		hana::make_pair(hana::type_c< double >, BOOST_HANA_STRING("float64"))
	);


}


#endif
