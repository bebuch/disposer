//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config__embedded_config__hpp_INCLUDED_
#define _disposer__config__embedded_config__hpp_INCLUDED_

#include "parse_config.hpp"

#include <map>
#include <set>
#include <tuple>


namespace disposer{


	class output_base;

	/// \brief Map from input names to output pointers
	using input_list = std::map< std::string, output_base* >;

	/// \brief Map from output names to count of connected inputs
	using output_list = std::map< std::string, std::size_t >;


	struct parameter_data{
		std::optional< std::string > generic_value;
		std::map< std::string, std::string > specialized_values;
	};

	using parameter_list = std::map< std::string, parameter_data >;



	namespace types::embedded_config{


		struct component{
			std::string name;
			std::string type_name;
			parameter_list parameters;
		};

		using components_config = std::vector< component >;


		using in = parse::in;

		struct out{
			std::string name;
			std::string variable;
			std::size_t use_count = 0;
		};

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


	types::embedded_config::config create_embedded_config(
		types::parse::config&& config
	);


}


#endif
