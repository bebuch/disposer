//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/config/unused_parameter_set_warning.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <map>


namespace disposer{


	void unused_parameter_set_warning(types::parse::config const& config){
		std::map< std::string, bool > parameter_sets;
		for(auto const& set: config.sets){
			parameter_sets.emplace(set.name, false);
		}

		for(auto const& component: config.components){
			for(auto const& set: component.parameters.parameter_sets){
				parameter_sets[set] = true;
			}
		}

		for(auto& chain: config.chains){
			for(auto const& module: chain.modules){
				for(auto const& set: module.parameters.parameter_sets){
					parameter_sets[set] = true;
				}
			}
		}

		for(auto const& set: parameter_sets){
			if(set.second) continue;

			logsys::log([&set](logsys::stdlogb& os){
				os << "parameter_set(" + set.first + ") is not used (WARNING)";
			});
		}
	}


}
