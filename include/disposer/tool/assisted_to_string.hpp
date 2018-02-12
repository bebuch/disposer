//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__assisted_to_string__hpp_INCLUDED_
#define _disposer__tool__assisted_to_string__hpp_INCLUDED_

#include "remove_optional.hpp"

#include <logsys/is_valid.hpp>

#include <sstream>
#include <utility>


namespace disposer::detail{


	struct valid_output_expr_t{
		template < typename T >
		auto operator()(T const& x)const
			->decltype((void)(std::declval< std::ostream& >() << x)){}
	};

	constexpr auto valid_output_expr = valid_output_expr_t{};


}


namespace disposer{


	template < typename T, typename =
		std::enable_if_t< logsys::is_valid< T >(detail::valid_output_expr) > >
	std::string to_string(T const& v){
		std::ostringstream os;
		os << v;
		return os.str();
	}

	inline std::string to_string(unsigned char v){
		return std::to_string(v);
	}

	inline std::string to_string(signed char v){
		return std::to_string(v);
	}


	template < typename T, bool Assert = false >
	std::string assisted_to_string(
		T const& v,
		hana::bool_< Assert > assert_ = hana::false_c
	){
		(void)assert_; // Silance GCC
		using std::begin;
		using std::end;

		if constexpr(
			logsys::is_valid< T >([](auto const& x)
				->decltype(std::string(to_string(x))){})
		){
			return to_string(v);
		}else if constexpr(detail::is_optional_v< T >){
			if(v){
				return "optional{" + assisted_to_string(*v, assert_) + "}";
			}else{
				return "optional{empty}";
			}
		}else if constexpr(
			auto const is_iterable = hana::is_valid(
				[](auto& x)->decltype(
						(void)begin(x) != end(x),
						(void)*begin(x),
						(void)++std::declval< decltype(begin(x)) >()
					){})(v);
			is_iterable
		){
			std::string result = "["
				+ std::to_string(std::distance(begin(v), end(v)))
				+ " elements]{";
			bool is_first = true;
			for(auto const& e: v){
				if(is_first){
					is_first = false;
				}else{
					result += ", ";
				}
				result += assisted_to_string(e, assert_);
			}
			result += "}";
		}else{
			static_assert(!Assert,
				"no to_string support for type T, provide an overload of:"
				"  S to_string(T const&)\n"
				"where S is convertible to std::string");
			return "(no output support)";
		}
	}


}


#endif
