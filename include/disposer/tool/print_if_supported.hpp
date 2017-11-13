//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__print_if_supported__hpp_INCLUDED_
#define _disposer__tool__print_if_supported__hpp_INCLUDED_

#include "remove_optional.hpp"

#include <ostream>
#include <utility>


namespace disposer{


	template < typename LogStream, typename T >
	void print_if_supported(LogStream& os, T const& v){
		if constexpr(detail::is_optional_v< T >){
			if(v){
				os << "{";
				print_if_supported(os, v);
				os << "}";
			}else{
				os << "{empty}";
			}
		}else{
			auto const is_printable_test = hana::is_valid([](auto& x)
				->decltype((void)(std::declval< std::ostream& >() << x)){});
			auto const is_printable = is_printable_test(v);
			if constexpr(is_printable){
				os << v;
			}else{
				auto const is_iterable = hana::is_valid(
					[](auto& x)->decltype((void)(x.begin() != x.end())){})(v);

				if constexpr(is_iterable){
					os << "[" << std::distance(v.begin(), v.end())
						<< " elements]{";
					auto const is_printable = is_printable_test(*v.begin());
					if constexpr(is_printable){
						bool is_first = true;
						for(auto const& e: v){
							if(is_first){
								is_first = false;
							}else{
								os << ", ";
							}
							print_if_supported(os, e);
						}
					}else{
						os << "(no output support)";
					}
					os << "}";
				}else{
					os << "(no output support)";
				}
			}
		}
	}


}


#endif
