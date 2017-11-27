//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__ct_pretty_name__hpp_INCLUDED_
#define _disposer__tool__ct_pretty_name__hpp_INCLUDED_

#include "type_index.hpp"

// #include <boost/algorithm/string/replace.hpp>

#include <vector>
// #include <regex>


namespace disposer{


// 	template < typename T >
// 	struct ct_pretty_name_t{
// 		std::string operator()()const{
// 			using boost::replace_all;
//
// 			std::string result = type_index::type_id< T >().pretty_name();
// 			replace_all(result, "float", "float32");
// 			replace_all(result, "long double", "-");
// 			replace_all(result, "double", "float64");
// 			if constexpr(std::is_same_v< unsigned char, std::uint8_t >){
// 				replace_all(result, "unsigned char", "#8");
// 				replace_all(result, "signed char", "+8");
// 			}
// 			if constexpr(std::is_same_v< unsigned long long, std::uint64_t >){
// 				replace_all(result, "unsigned long long", "#64");
// 				replace_all(result, "long long", "+64");
// 			}
// 			if constexpr(std::is_same_v< unsigned long, std::uint64_t >){
// 				replace_all(result, "unsigned long", "#64");
// 				std::regex expr("(?![a-zA-Z0-9_])long(?![a-zA-Z0-9_])");
// 				result = std::regex_replace(result, expr, "+64");
// // 				replace_all(result, "long", "+64");
// 			}
// 			if constexpr(std::is_same_v< unsigned long, std::uint32_t >){
// 				replace_all(result, "unsigned long", "#32");
// 				std::regex expr("(?![a-zA-Z0-9_])long(?![a-zA-Z0-9_])");
// 				result = std::regex_replace(result, expr, "+32");
// // 				replace_all(result, "long", "+32");
// 			}
// 			if constexpr(std::is_same_v< unsigned, std::uint32_t >){
// 				replace_all(result, "unsigned int", "#32");
// 				std::regex expr("(?![a-zA-Z0-9_])int(?![a-zA-Z0-9_])");
// 				result = std::regex_replace(result, expr, "+32");
// // 				replace_all(result, "int", "+32");
// 			}
// 			if constexpr(std::is_same_v< unsigned short, std::uint16_t >){
// 				replace_all(result, "unsigned short", "#16");
// 				std::regex expr("(?![a-zA-Z0-9_])long(?![a-zA-Z0-9_])");
// 				result = std::regex_replace(result, expr, "+16");
// // 				replace_all(result, "short", "+16");
// 			}
// 			replace_all(result, "+", "int");
// 			replace_all(result, "#", "uint");
// 			replace_all(result, "-", "long double");
// 			return result;
// 		}
// 	};

	template < typename T >
	struct ct_pretty_name_t{
		std::string operator()()const{
			return type_index::type_id< T >().pretty_name();
		}
	};

	template < typename T >
	struct ct_pretty_name_t< std::vector< T > >{
		std::string operator()()const{
			return "std::vector<" + ct_pretty_name_t< T >{}() + ">";
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
