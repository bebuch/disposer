//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension_referrer_output__hpp_INCLUDED_
#define _disposer__core__dimension_referrer_output__hpp_INCLUDED_

#include "dimension_referrer.hpp"

#include "../tool/ct_pretty_name.hpp"

#include <io_tools/make_string.hpp>


namespace disposer{


	/// \brief Print all possible combinations of a dimension_referrer object
	template <
		template < typename ... > typename Template,
		std::size_t ... Ds,
		typename ... DTs >
	std::string wrapped_type_ref_text(
		dimension_referrer< Template, Ds ... >,
		dimension_list< DTs ... >
	){
		if constexpr(is_self_t_v< Template >){
			return "      * type is equal to dimension "
				+ std::to_string((Ds, ...) + 1) + "\n";
		}else if constexpr(sizeof...(Ds) > 0){
			std::string text = "      * type depends on dimension: ";
			dimension_converter< dimension_list< DTs ... >, Template, Ds ... >
				convert;
			hana::unpack(convert.packed, [&text](auto ... i){
					text += io_tools::make_string_separated_by(", ",
						(1 + std::size_t(i)) ...) + "\n";
				});

			hana::for_each(convert.indexes, [&text, convert](auto index){
					text += "        * ";
					hana::unpack(convert.packed, [&text](auto ... p){
							text += io_tools::make_string_separated_by(
								" × ",
								ct_pretty_name<
									typename decltype(
										+dimension_list< DTs ... >::
										dimensions[p]
										[index[convert.pos(p)]])::type
								 >() ...);
						});

					text += " => " + ct_pretty_name< typename
							decltype(convert.value_type_of(index))::type
						>() + "\n";
				});

			return text;
		}else{
			return "      * type: "
				+ ct_pretty_name< Template<> >() + "\n";
		}
	}


}


#endif
