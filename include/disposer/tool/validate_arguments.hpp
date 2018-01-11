//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__validate_arguments__hpp_INCLUDED_
#define _disposer__tool__validate_arguments__hpp_INCLUDED_

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/or.hpp>


namespace disposer{


	namespace hana = boost::hana;


}


namespace disposer::detail{


	template < typename ... ValidTags >
	struct argument_validator{
		template < typename Arg >
		constexpr void validate(Arg const& arg)const{
			auto const is_valid = hana::or_(hana::is_a< ValidTags >(arg) ...);
			static_assert(is_valid, "argument is not valid");
		}

		template < typename ... Args >
		constexpr void operator()(Args const& ... args)const{
			(validate(args), ..., (void)0);
		}
	};

	template < typename ... ValidTags >
	constexpr argument_validator< ValidTags ... > validate_arguments{};


}


#endif
