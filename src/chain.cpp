//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/chain.hpp>
#include <disposer/module_base.hpp>

#include <numeric>


namespace disposer{


	chain::chain(
		std::vector< module_ptr >&& modules,
		id_generator& generate_id,
		std::string const& name,
		std::string const& group
	):
		name(name),
		group(group),
		modules_(std::move(modules)),
		id_increase_(std::accumulate(
			modules_.cbegin(),
			modules_.cend(),
			std::size_t(1),
			[](std::size_t increase, module_ptr const& module){
				return increase * module->id_increase;
			}
		)),
		generate_id_(generate_id),
		next_run_(0),
		ready_run_(modules_.size()),
		mutexes_(modules_.size())
		{}


	void chain::exec(){
		std::size_t const id = generate_id_(id_increase_);
		std::size_t const run = next_run_++;

		log([this, id](log_base& os){
			os << "id(" << id << ") chain '" << modules_[0]->chain << "'";
		}, [this, id, run]{
			try{
				for(std::size_t i = 0; i < modules_.size(); ++i){
					modules_[i]->set_id(id);
					process_module(i, run, [](chain& c, std::size_t i){
						c.modules_[i]->exec();
					}, "exec");
				}
			}catch(...){
				// cleanup and unlock all executions
				for(std::size_t i = 0; i < ready_run_.size(); ++i){
					// exec was successful
					if(ready_run_[i] >= run + 1) continue;

					process_module(i, run, [id](chain& c, std::size_t i){
						c.modules_[i]->cleanup(id);
					}, "cleanup");
				}

				// rethrow exception
				throw;
			}
		});
	}

	template < typename F >
	void chain::process_module(
		std::size_t i,
		std::size_t run,
		F const& action,
		char const* action_name
	){
		// Lock mutex and wait for the previous run to be ready
		std::unique_lock< std::mutex > lock(mutexes_[i]);
		cv_.wait(lock, [this, i, run]{ return ready_run_[i] == run; });

		// Cleanup the module
		log([this, i, action_name](log_base& os){
			os << "id(" << modules_[i]->id << "." << i << ") " << action_name
				<< " chain '" << modules_[i]->chain << "' module '"
				<< modules_[i]->name << "'";
		}, [this, i, action]{ action(*this, i); });

		// Make module ready
		ready_run_[i] = run + 1;
		cv_.notify_all();
	}



}
