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

#ifdef DISPOSER_BITMAP_PIXEL_AS_TEXT
#include <bitmap/pixel.hpp>
#endif

#include <cstdint>
#include <string>


namespace disposer{


	namespace hana = boost::hana;

	constexpr auto as_text = hana::make_map
		(hana::make_pair(hana::type_c< bool >, BOOST_HANA_STRING("bool"))
		,hana::make_pair(hana::type_c< char >, BOOST_HANA_STRING("char"))
		,hana::make_pair(hana::type_c< std::int8_t >,
			BOOST_HANA_STRING("sint8"))
		,hana::make_pair(hana::type_c< std::uint8_t >,
			BOOST_HANA_STRING("uint8"))
		,hana::make_pair(hana::type_c< std::int16_t >,
			BOOST_HANA_STRING("sint16"))
		,hana::make_pair(hana::type_c< std::uint16_t >,
			BOOST_HANA_STRING("uint16"))
		,hana::make_pair(hana::type_c< std::int32_t >,
			BOOST_HANA_STRING("sint32"))
		,hana::make_pair(hana::type_c< std::uint32_t >,
			BOOST_HANA_STRING("uint32"))
		,hana::make_pair(hana::type_c< std::int64_t >,
			BOOST_HANA_STRING("sint64"))
		,hana::make_pair(hana::type_c< std::uint64_t >,
			BOOST_HANA_STRING("uint64"))
		,hana::make_pair(hana::type_c< float >,  BOOST_HANA_STRING("float32"))
		,hana::make_pair(hana::type_c< double >, BOOST_HANA_STRING("float64"))
		,hana::make_pair(hana::type_c< std::string >,
			BOOST_HANA_STRING("string"))
#ifdef DISPOSER_BITMAP_PIXEL_AS_TEXT
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga8 >,
			BOOST_HANA_STRING("ga8"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga16 >,
			BOOST_HANA_STRING("ga16"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga32 >,
			BOOST_HANA_STRING("ga32"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga64 >,
			BOOST_HANA_STRING("ga64"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga8u >,
			BOOST_HANA_STRING("ga8u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga16u >,
			BOOST_HANA_STRING("ga16u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga32u >,
			BOOST_HANA_STRING("ga32u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga64u >,
			BOOST_HANA_STRING("ga64u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga32f >,
			BOOST_HANA_STRING("ga32f"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::ga64f >,
			BOOST_HANA_STRING("ga64f"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb8 >,
			BOOST_HANA_STRING("rgb8"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb16 >,
			BOOST_HANA_STRING("rgb16"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb32 >,
			BOOST_HANA_STRING("rgb32"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb64 >,
			BOOST_HANA_STRING("rgb64"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb8u >,
			BOOST_HANA_STRING("rgb8u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb16u >,
			BOOST_HANA_STRING("rgb16u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb32u >,
			BOOST_HANA_STRING("rgb32u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb64u >,
			BOOST_HANA_STRING("rgb64u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb32f >,
			BOOST_HANA_STRING("rgb32f"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgb64f >,
			BOOST_HANA_STRING("rgb64f"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba8 >,
			BOOST_HANA_STRING("rgba8"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba16 >,
			BOOST_HANA_STRING("rgba16"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba32 >,
			BOOST_HANA_STRING("rgba32"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba64 >,
			BOOST_HANA_STRING("rgba64"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba8u >,
			BOOST_HANA_STRING("rgba8u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba16u >,
			BOOST_HANA_STRING("rgba16u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba32u >,
			BOOST_HANA_STRING("rgba32u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba64u >,
			BOOST_HANA_STRING("rgba64u"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba32f >,
			BOOST_HANA_STRING("rgba32f"))
		,hana::make_pair(hana::type_c< ::bitmap::pixel::rgba64f >,
			BOOST_HANA_STRING("rgba64f"))
#endif
	);


}


#endif
