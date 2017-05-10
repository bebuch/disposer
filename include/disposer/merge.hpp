//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config_merge_parameter__hpp_INCLUDED_
#define _disposer__config_merge_parameter__hpp_INCLUDED_

#include "parse.hpp"

#include <map>


namespace disposer{


	struct parameter_data{
		std::optional< std::string > generic_value;
		std::map< std::string, std::string > specialized_values;
	};


	namespace types::merge{


		using in = parse::in;
		using out = parse::out;

		struct module{
			std::string type_name;
			std::map< std::string, parameter_data > parameters;
			std::vector< in > inputs;
			std::vector< out > outputs;
		};

		struct chain{
			std::string name;
			std::string id_generator;
			std::vector< module > modules;
		};

		using config = std::vector< chain >;


	}


	types::merge::config merge(types::parse::config&& config);


}


#endif
