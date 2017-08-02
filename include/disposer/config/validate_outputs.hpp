//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__validate_outputs__hpp_INCLUDED_
#define _disposer__validate_outputs__hpp_INCLUDED_

#include "embedded_config.hpp"

#include "../tool/to_std_string.hpp"

#include <boost/hana/transform.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/filter.hpp>

#include <algorithm>


namespace disposer{


	namespace hana = boost::hana;


	template < typename Makers >
	output_list validate_outputs(
		std::string const& location,
		Makers const& makers,
		output_list const& outputs
	){
		auto output_names = hana::transform(
			hana::filter(makers, hana::is_a< output_maker_tag >),
			[](auto const& output_maker){
				return output_maker.name;
			});

		auto output_name_list = outputs;
		hana::for_each(output_names,
			[&output_name_list](auto const& name){
				output_name_list.erase(detail::to_std_string(name));
			});

		for(auto const& out: output_name_list){
			logsys::log([&location, &out](logsys::stdlogb& os){
				os << location << "output("
					<< out << ") doesn't exist (ERROR)";
			});
		}
		return output_name_list;
	}


}


#endif
