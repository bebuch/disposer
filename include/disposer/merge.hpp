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
#include <set>
#include <tuple>


namespace disposer{


	class output_base;

	/// \brief Map from input names to output pointers
	using input_list =
		std::map< std::string, std::tuple< output_base*, bool > >;

	/// \brief Output name lists
	using output_list = std::set< std::string >;


	struct parameter_data{
		std::optional< std::string > generic_value;
		std::map< std::string, std::string > specialized_values;
	};

	using parameter_list = std::map< std::string, parameter_data >;



	namespace types::merge{


		struct component{
			std::string name;
			std::string type_name;
			parameter_list parameters;
		};

		using components_config = std::vector< component >;


		using in = parse::in;
		using out = parse::out;

		struct module{
			std::string type_name;
			parameter_list parameters;
			std::vector< in > inputs;
			std::vector< out > outputs;
		};

		struct chain{
			std::string name;
			std::string id_generator;
			std::vector< module > modules;
		};

		using chains_config = std::vector< chain >;

		struct config{
			components_config components;
			chains_config chains;
		};


	}


	types::merge::config merge(types::parse::config&& config);


}


#endif
