//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__validate_parameters__hpp_INCLUDED_
#define _disposer__config__validate_parameters__hpp_INCLUDED_

#include "embedded_config.hpp"

#include "../tool/to_std_string.hpp"

#include <boost/hana/transform.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/filter.hpp>

#include <algorithm>


namespace disposer{


	namespace hana = boost::hana;


	template < typename Makers >
	std::set< std::string > validate_parameters(
		std::string const& location,
		Makers const& makers,
		parameter_list const& params
	){
		auto parameters_names = hana::transform(
			hana::filter(makers, hana::is_a< parameter_maker_tag >),
			[](auto const& parameters_maker){
				return parameters_maker.name;
			});

		std::set< std::string > parameter_name_list;
		std::transform(params.begin(), params.end(),
			std::inserter(parameter_name_list, parameter_name_list.end()),
			[](auto const& pair){ return pair.first; });
		hana::for_each(parameters_names,
			[&parameter_name_list](auto const& name){
				parameter_name_list.erase(detail::to_std_string(name));
			});

		for(auto const& param: parameter_name_list){
			logsys::log([&location, &param](logsys::stdlogb& os){
				os << location << "parameter("
					<< param << ") doesn't exist (WARNING)";
			});
		}

		return parameter_name_list;
	}


}


#endif
