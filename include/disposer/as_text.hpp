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

#include <string_view>
#include <cstdint>


namespace disposer{


	using namespace std::literals::string_view_literals;

	namespace hana = boost::hana;


	constexpr auto as_text = hana::make_map(
		hana::make_pair(hana::type_c< bool >, "bool"sv),
		hana::make_pair(hana::type_c< char >, "char"sv),
		hana::make_pair(hana::type_c< std::int8_t >,   "sint8"sv),
		hana::make_pair(hana::type_c< std::uint8_t >,  "uint8"sv),
		hana::make_pair(hana::type_c< std::int16_t >,  "sint16"sv),
		hana::make_pair(hana::type_c< std::uint16_t >, "uint16"sv),
		hana::make_pair(hana::type_c< std::int32_t >,  "sint32"sv),
		hana::make_pair(hana::type_c< std::uint32_t >, "uint32"sv),
		hana::make_pair(hana::type_c< std::int64_t >,  "sint64"sv),
		hana::make_pair(hana::type_c< std::uint64_t >, "uint64"sv),
		hana::make_pair(hana::type_c< float >,  "float32"sv),
		hana::make_pair(hana::type_c< double >, "float64"sv)
	);


}


#endif
