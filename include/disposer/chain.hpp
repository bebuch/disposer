//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_chain_hpp_INCLUDED_
#define _disposer_chain_hpp_INCLUDED_

#include "module_base.hpp"
#include "log_base.hpp"
#include "log.hpp"

#include <mutex>
#include <string>
#include <vector>
#include <condition_variable>


namespace disposer{


	class chain;
	using chain_list = std::unordered_map< std::string, chain >;

	namespace config{

		chain_list load(std::string const& filename);

	}

	/* Eigenschaften:
	* 
	* - keine 2 identischen Module (Trigger) laufen gleichzeitig
	* - es darf nicht überholt werden
	*/
	class chain{
	public:
		chain(std::vector< module_ptr >&& modules, std::size_t counter_increase):
			counter_increase(counter_increase),
			modules(std::move(modules)),
			next_id{},
			ready_id(modules.size()),
			mutexes(modules.size())
			{}

		chain(chain&& c):
			counter_increase(c.counter_increase),
			modules(std::move(c.modules)),
			next_id{},
			ready_id(modules.size()),
			mutexes(modules.size())
			{}


		void trigger(){
			// Get ID and increase next_id
			std::size_t id = [this]{
				std::lock_guard< std::mutex > lock(mutex);
				return next_id++;
			}();

			log([this, id](log_base& os){
				os << "id(" << id << ") chain '" << modules[0]->chain << "'";
			}, [this, id]{
				try{
					for(std::size_t i = 0; i < modules.size(); ++i){
						process_module(i, id, [this, i, id]{ modules[i]->trigger(id); }, "trigger");
					}
				}catch(...){
					// cleanup and unlock all triggers
					for(std::size_t i = 0; i < ready_id.size(); ++i){
						// Trigger was successful
						if(ready_id[i] >= id + 1) continue;

						process_module(i, id, [this, i, id]{ modules[i]->cleanup(id); }, "cleanup");
					}

					// rethrow exception
					throw;
				}
			});
		}

	private:
		template < typename ActionName, typename Action >
		void process_module(std::size_t i, std::size_t id, Action&& action, ActionName action_name){
			// Lock mutex and wait for the previous id to be ready
			std::unique_lock< std::mutex > lock(mutexes[i]);
			cv.wait(lock, [this, i, id]{return ready_id[i] == id;});

			// Cleanup the module
			log([this, i, id, action_name](log_base& os){
				os << "id(" << id << "." << i << ") " << action_name << " chain '" << modules[i]->chain << "' module '" << modules[i]->name << "'";
			}, action);

			// Make module ready
			{
				std::lock_guard < std::mutex > lock(mutex);
				ready_id[i] = id + 1;
			}
			cv.notify_all();
		}

		std::size_t counter_increase;
		std::vector< module_ptr > modules;

		std::size_t next_id;

		std::vector< std::size_t > ready_id;
		std::vector< std::mutex > mutexes;
		std::mutex mutex;
		std::condition_variable cv;

		friend chain_list disposer::config::load(std::string const& filename);
	};


}


#endif
