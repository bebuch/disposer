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
#include <disposer/create_chain_modules.hpp>

#include <numeric>


namespace disposer{


	chain::chain(
		module_maker_list const& maker_list,
		types::merge::chain const& config_chain,
		id_generator& generate_id,
		std::string const& group
	):
		name(config_chain.name),
		group(group),
		modules_(create_chain_modules(maker_list, config_chain)),
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
		// generate a new id for the exec
		std::size_t const id = generate_id_(id_increase_);

		// generate a unique continuous index for the call
		std::size_t const run = next_run_++;

		// exec any module, call cleanup instead if the module throw
		log([this, id](log_base& os){
			os << "id(" << id << ") chain '" << modules_[0]->chain << "'";
		}, [this, id, run]{
			try{
				for(std::size_t i = 0; i < modules_.size(); ++i){
					modules_[i]->set_id(chain_key(), id);
					process_module(i, run, [](chain& c, std::size_t i){
						c.modules_[i]->exec(chain_key());
					}, "exec");
				}
			}catch(...){
				// cleanup and unlock all executions
				for(std::size_t i = 0; i < ready_run_.size(); ++i){
					// exec was successful
					if(ready_run_[i] >= run + 1) continue;

					process_module(i, run, [id](chain& c, std::size_t i){
						c.modules_[i]->cleanup(chain_key(), id);
					}, "cleanup");
				}

				// rethrow exception
				throw;
			}
		});
	}

	template < typename F >
	void chain::process_module(
		std::size_t const i,
		std::size_t const run,
		F const& action,
		char const* const action_name
	){
		// lock mutex and wait for the previous run to be ready
		std::unique_lock< std::mutex > lock(mutexes_[i]);
		cv_.wait(lock, [this, i, run]{ return ready_run_[i] == run; });

		// exec or cleanup the module
		log([this, i, action_name](log_base& os){
			os << "id(" << modules_[i]->id << "." << i << ") " << action_name
				<< " chain '" << modules_[i]->chain << "' module '"
				<< modules_[i]->name << "'";
		}, [this, i, &action]{ action(*this, i); });

		// make module ready
		ready_run_[i] = run + 1;
		cv_.notify_all();
	}



}
