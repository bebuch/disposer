//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/unused_warnings.hpp>

#include <logsys/log_base.hpp>
#include <logsys/log.hpp>

#include <map>


namespace disposer{


	void unused_warnings(types::parse::config const& config){
		std::map< std::string, bool > parameter_sets;
		for(auto& set: config.sets){
			parameter_sets.emplace(set.name, false);
		}

		std::map< std::string, bool > modules;
		for(auto& module: config.modules){
			modules.emplace(module.name, false);

			for(auto& set: module.parameter_sets){
				parameter_sets[set] = true;
			}
		}

		for(auto const& set: parameter_sets){
			if(set.second) continue;

			logsys::log([&set](logsys::log_base& os){
				os << "parameter_set '" + set.first + "' is not used";
			});
		}

		for(auto& chain: config.chains){
			std::map< std::string, bool > variables;
			for(auto& module: chain.modules){
				modules[module.name] = true;

				for(auto& input: module.inputs){
					variables[input.variable] = true;
				}

				for(auto& output: module.outputs){
					variables.emplace(output.variable, false);
				}
			}

			for(auto const& var: variables){
				if(var.second) continue;

				logsys::log([&var, &chain](logsys::log_base& os){
					os << "In chain '" << chain.name << "': variable '"
						<< var.first << "' is not used";
				});
			}
		}

		for(auto const& module: modules){
			if(module.second) continue;

			logsys::log([&module](logsys::log_base& os){
				os << "module '" + module.first + "' is not used";
			});
		}
	}


}
