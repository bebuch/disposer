//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__validate_iop__hpp_INCLUDED_
#define _disposer__config__validate_iop__hpp_INCLUDED_

#include "../tool/to_std_string.hpp"
#include "../tool/false_c.hpp"

#include <boost/hana/transform.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/filter.hpp>

#include <algorithm>
#include <set>


namespace disposer{


	namespace hana = boost::hana;

	struct input_maker_tag;
	struct output_maker_tag;
	struct parameter_maker_tag;


	template < typename MakerTag, typename Configuration, typename List >
	std::set< std::string > validate_iop(
		std::string const& location,
		Configuration const& configuration,
		List const& list
	){
		auto names = hana::transform(
			hana::filter(configuration.config_list, hana::is_a< MakerTag >),
			[](auto const& maker){ return maker.name; });

		std::set< std::string > name_list;
		std::transform(list.begin(), list.end(),
			std::inserter(name_list, name_list.end()),
			[](auto const& pair){ return pair.first; });
		hana::for_each(names,
			[&name_list](auto const& name){
				name_list.erase(detail::to_std_string(name));
			});

		for(auto const& name: name_list){
			logsys::log([&location, &name](logsys::stdlogb& os){
				os << location;
				if constexpr(
					std::is_same_v< MakerTag, input_maker_tag >
				){
					os << "input";
				}else if constexpr(
					std::is_same_v< MakerTag, output_maker_tag >
				){
					os << "output";
				}else if constexpr(
					std::is_same_v< MakerTag, parameter_maker_tag >
				){
					os << "parameter";
				}else{
					static_assert(detail::false_c< MakerTag >,
						"MakerTag must be input_maker_tag, output_maker_tag "
						"or parameter_maker_tag");
				}
				os << "(" << name << ") doesn't exist";
				if constexpr(std::is_same_v< MakerTag, parameter_maker_tag >){
					os << " (WARNING)";
				}else{
					os << " (ERROR)";
				}
			});
		}
		return name_list;
	}


}


#endif
