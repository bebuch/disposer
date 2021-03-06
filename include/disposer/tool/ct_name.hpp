//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__ct_name__hpp_INCLUDED_
#define _disposer__tool__ct_name__hpp_INCLUDED_

#include <boost/hana/string.hpp>

#include <array>
#include <type_traits>


namespace disposer{


	namespace hana = boost::hana;


	/// \brief true if name contains only valid characters ([a-zA-Z0-9_])
	template < std::size_t N >
	constexpr bool valid_name(std::array< char, N > const& name){
		for(auto c: name){
			if(!(
				(c >= 'A' && c <= 'Z') ||
				(c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') ||
				c == '_'
			)) return false;
		}
		return true;
	}


	/// \brief A compile time string type containing only valid characters
	///        ([a-zA-Z0-9_])
	template < typename Tag, char ... C >
	struct ct_name{
		static_assert(valid_name(std::array< char, sizeof...(C) >{{ C ... }}),
			"only [A-Za-z0-9_] are valid characters");

		/// \brief Detection type for hana::is_a
		using hana_tag = Tag;

		/// \brief The string as hana::string object
		static constexpr auto value = hana::string_c< C ... >;

		/// \brief The string as hana::string type
		using value_type = std::remove_cv_t< decltype(value) >;
	};


	template < typename Tag, char ... C >
	constexpr hana::true_ operator==(
		ct_name< Tag, C ... >,
		ct_name< Tag, C ... >
	)noexcept{ return {}; }

	template < typename TagA, char ... CA, typename TagB, char ... CB >
	constexpr hana::false_ operator==(
		ct_name< TagA, CA ... >,
		ct_name< TagB, CB ... >
	)noexcept{ return {}; }


	template < typename Tag, char ... C >
	constexpr hana::false_ operator!=(
		ct_name< Tag, C ... >,
		ct_name< Tag, C ... >
	)noexcept{ return {}; }

	template < typename TagA, char ... CA, typename TagB, char ... CB >
	constexpr hana::true_ operator!=(
		ct_name< TagA, CA ... >,
		ct_name< TagB, CB ... >
	)noexcept{ return {}; }


}


#endif
