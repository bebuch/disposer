//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__chain__hpp_INCLUDED_
#define _disposer__chain__hpp_INCLUDED_

#include "module_base.hpp"
#include "id_generator.hpp"
#include "log_base.hpp"
#include "log.hpp"


#include <mutex>
#include <string>
#include <vector>
#include <condition_variable>


namespace disposer{


	/* Eigenschaften:
	* 
	* - keine 2 identischen Module (Trigger) laufen gleichzeitig
	* - es darf nicht überholt werden
	*/
	class chain{
	public:
		chain(std::vector< module_ptr >&& modules, id_generator& generate_id, std::size_t counter_increase);

		chain(chain const&) = delete;
		chain(chain&&) = delete;

		chain& operator=(chain const&) = delete;
		chain& operator=(chain&&) = delete;


		void trigger();


	private:
		template < typename F >
		void process_module(std::size_t i, std::size_t run, F const& action, char const* action_name);

		std::vector< module_ptr > modules_;

		std::size_t counter_increase_;
		id_generator& generate_id_;

		std::atomic< std::size_t > next_run_;
		std::vector< std::size_t > ready_run_;

		std::vector< std::mutex > mutexes_;
		std::condition_variable cv_;
	};


}


#endif
