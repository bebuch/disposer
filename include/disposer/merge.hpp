//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_config_merge_parameter_hpp_INCLUDED_
#define _disposer_config_merge_parameter_hpp_INCLUDED_

#include "parse.hpp"

#include <map>


namespace disposer{


	namespace types{ namespace merge{


		struct module{
			std::string type;
			std::map< std::string, std::string > parameters;
		};

		using modules = std::map< std::string, module >;


		struct chain_module{
			std::pair< std::string const, merge::module >& module;
			std::vector< parse::io > inputs;
			std::vector< parse::io > outputs;
		};

		struct chain{
			std::string name;
			std::string id_generator;
			std::size_t increase;
			std::vector< chain_module > modules;
		};

		using chains = std::vector< chain >;

		struct config{
			merge::modules modules;
			merge::chains chains;
		};


	} }


	types::merge::config merge(types::parse::config&& config);


}


#endif
