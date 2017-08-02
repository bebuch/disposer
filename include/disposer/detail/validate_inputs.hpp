//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__validate_inputs__hpp_INCLUDED_
#define _disposer__validate_inputs__hpp_INCLUDED_

#include "embedded_config.hpp"
#include "to_std_string.hpp"

#include <boost/hana/transform.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/filter.hpp>

#include <algorithm>


namespace disposer{


	namespace hana = boost::hana;


	template < typename Makers >
	std::set< std::string > validate_inputs(
		std::string const& location,
		Makers const& makers,
		input_list const& inputs
	){
		auto input_names = hana::transform(
			hana::filter(makers, hana::is_a< input_maker_tag >),
			[](auto const& input_maker){
				return input_maker.name;
			});

		std::set< std::string > input_name_list;
		std::transform(inputs.begin(), inputs.end(),
			std::inserter(input_name_list, input_name_list.end()),
			[](auto const& pair){ return pair.first; });
		hana::for_each(input_names,
			[&input_name_list](auto const& name){
				input_name_list.erase(detail::to_std_string(name));
			});

		for(auto const& in: input_name_list){
			logsys::log([&location, &in](logsys::stdlogb& os){
				os << location << "input("
					<< in << ") doesn't exist (ERROR)";
			});
		}
		return input_name_list;
	}


}


#endif
