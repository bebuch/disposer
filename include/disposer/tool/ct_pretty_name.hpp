//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__ct_pretty_name__hpp_INCLUDED_
#define _disposer__tool__ct_pretty_name__hpp_INCLUDED_

#include "type_index.hpp"

#include <vector>
#include <optional>


namespace disposer{


	std::string type_print(std::string const& type);

	std::string type_print(type_index const& type);


	template < typename T >
	struct ct_pretty_name_t{
		std::string operator()()const{
			return type_print(type_index::type_id< T >());
		}
	};

	template < typename T >
	struct ct_pretty_name_t< std::vector< T > >{
		std::string operator()()const{
			return "std::vector<" + ct_pretty_name_t< T >{}() + ">";
		}
	};

	template < typename T >
	struct ct_pretty_name_t< std::optional< T > >{
		std::string operator()()const{
			return "std::optional<" + ct_pretty_name_t< T >{}() + ">";
		}
	};

	template <>
	struct ct_pretty_name_t< std::string >{
		std::string operator()()const{
			return "std::string";
		}
	};


	template < typename T >
	constexpr ct_pretty_name_t< T > ct_pretty_name{};


}


#endif
