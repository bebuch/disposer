//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__ct_name__hpp_INCLUDED_
#define _disposer__ct_name__hpp_INCLUDED_

#include <boost/hana.hpp>

#include <array>
#include <string_view>
#include <string>


namespace disposer{


	namespace hana = boost::hana;


	/// \brief Converts a hana::string to std::string_view
	template < typename String,
		typename = std::enable_if< hana::is_a< hana::string_tag, String >() > >
	constexpr std::string_view to_std_string_view(String const& s)noexcept{
		return {s.c_str(), hana::size(s)};
	}

	/// \brief Converts a hana::string to std::string
	template < typename String,
		typename = std::enable_if< hana::is_a< hana::string_tag, String >() > >
	std::string to_std_string(String const& s){
		return {s.c_str(), hana::size(s)};
	}


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
	template < char ... C >
	struct ct_name{
		static_assert(valid_name(std::array< char, sizeof...(C) >{{ C ... }}),
			"only [A-Za-z0-9_] are valid characters");

		/// \brief The string as hana::string object
		static constexpr auto value = hana::string_c< C ... >;

		/// \brief The string as hana::string type
		using value_type = std::remove_cv_t< decltype(value) >;
	};


}


#endif
