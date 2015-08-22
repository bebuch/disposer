//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_config_parse_hpp_INCLUDED_
#define _disposer_config_parse_hpp_INCLUDED_

#include <boost/optional.hpp>

#include <string>
#include <vector>


namespace disposer{


	namespace types{ namespace parse{


		struct parameter{
			std::string key;
			std::string value;
		};

		struct parameter_set{
			std::string name;
			std::vector< parameter > parameters;
		};

		using parameter_sets = std::vector< parameter_set >;


		struct module{
			std::string name;
			std::string type_name;
			std::vector< std::string > parameter_sets;
			std::vector< parameter > parameters;
		};

		using modules = std::vector< module >;


		struct io{
			std::string name;
			std::string variable;
		};

		struct chain_module{
			std::string name;
			std::vector< io > inputs;
			std::vector< io > outputs;
		};

		struct chain{
			std::string name;
			boost::optional< std::size_t > increase;
			boost::optional< std::string > id_generator;
			std::vector< chain_module > modules;
		};

		using chains = std::vector< chain >;


		struct config{
			parse::parameter_sets sets;
			parse::modules modules;
			parse::chains chains;
		};


	} }


	types::parse::config parse(std::istream& is);
	types::parse::config parse(std::string const& filename);


}


#endif
