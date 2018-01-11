//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__to_std_string_view__hpp_INCLUDED_
#define _disposer__tool__to_std_string_view__hpp_INCLUDED_

#include "ct_name.hpp"

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/size.hpp>

#include <string_view>



namespace disposer::detail{


	/// \brief Converts a hana::string to std::string_view
	template < typename String, typename
		= std::enable_if_t< hana::is_a< hana::string_tag, String >() > >
	constexpr std::string_view to_std_string_view(String const& s)noexcept{
		return {s.c_str(), hana::size(s)};
	}

	/// \brief Converts a disposer::ct_name to std::string_view
	template < typename Tag, char ... C >
	constexpr std::string_view to_std_string_view(
		disposer::ct_name< Tag, C ... > const& name
	)noexcept{
		return to_std_string_view(name.value);
	}


}


#endif
