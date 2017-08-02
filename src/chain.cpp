//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/detail/module_base.hpp>

#include <disposer/chain.hpp>

#include <disposer/config/create_chain_modules.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <numeric>


namespace disposer{


	/// \brief Class disposer access key
	struct chain_key{
	private:
		/// \brief Constructor
		constexpr chain_key()noexcept = default;
		friend class chain;
	};


	chain::chain(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain,
		id_generator& generate_id
	):
		name(config_chain.name),
		modules_(create_chain_modules(maker_list, config_chain)),
		generate_id_(generate_id),
		next_run_(0),
		ready_run_(modules_.size()),
		module_mutexes_(modules_.size()),
		enabled_(false),
		exec_calls_count_(0)
		{}


	chain::~chain(){
		disable();
	}


	namespace{


		class exec_call_manager{
		public:
			exec_call_manager(
				std::atomic< std::size_t >& exec_calls_count,
				std::condition_variable& enable_cv
			):
				exec_calls_count_(exec_calls_count),
				enable_cv_(enable_cv){
					++exec_calls_count_;
				}

			~exec_call_manager(){
				--exec_calls_count_;
				enable_cv_.notify_all();
			}

		private:
			std::atomic< std::size_t >& exec_calls_count_;
			std::condition_variable& enable_cv_;
		};



	}


	void chain::exec(){
		if(!enabled_){
			throw std::logic_error("chain(" + name + ") is not enabled");
		}

		exec_call_manager lock(exec_calls_count_, enable_cv_);

		// generate a new id for the exec
		std::size_t const id = generate_id_();

		// generate a unique continuous index for the call
		std::size_t const run = next_run_++;

		// exec any module, call cleanup instead if the module throw
		logsys::log([this, id](logsys::stdlogb& os){
			os << "id(" << id << ") chain(" << name << ")";
		}, [this, id, run]{
			try{
				for(std::size_t i = 0; i < modules_.size(); ++i){
					modules_[i]->set_id(chain_key(), id);
					process_module(i, run, [this, id](std::size_t i){
						modules_[i]->exec(chain_key());
						modules_[i]->cleanup(chain_key(), id);
					}, "exec");
				}
			}catch(...){
				// cleanup and unlock all executions
				for(std::size_t i = 0; i < ready_run_.size(); ++i){
					// exec was successful
					if(ready_run_[i] >= run + 1) continue;

					process_module(i, run, [this, id](std::size_t i){
						modules_[i]->cleanup(chain_key(), id);
					}, "cleanup");
				}

				// rethrow exception
				throw;
			}
		});
	}


	void chain::enable(){
		std::unique_lock< std::mutex > lock(enable_mutex_);
		if(enabled_) return;

		enable_cv_.wait(lock, [this]{ return exec_calls_count_ == 0; });

		logsys::log(
			[this](logsys::stdlogb& os){
				os << "chain(" << name << ") enabled";
			},
			[this]{
				std::size_t i = 0;
				try{
					// enable all modules
					for(; i < modules_.size(); ++i){
						logsys::log([this, i](logsys::stdlogb& os){
								os << "chain(" << name << ") module("
									<< modules_[i]->number << ":"
									<< modules_[i]->type_name << ") enabled";
							}, [this, i]{
								modules_[i]->enable(chain_key());
							});
					}
				}catch(...){
					// disable all modules until the one who throw
					for(std::size_t j = 0; j < i; ++j){
						logsys::log([this, i, j](logsys::stdlogb& os){
								os << "chain(" << name << ") module("
									<< modules_[j]->number
									<< ") disabled because of exception while "
									<< "enable module(" << modules_[i]->number
									<< ")";
							}, [this, j]{
								modules_[j]->disable(chain_key());
							});
					}

					// rethrow exception
					throw;
				}
			});

		enabled_ = true;
	}


	void chain::disable()noexcept{
		std::unique_lock< std::mutex > lock(enable_mutex_);
		if(!enabled_.exchange(false)) return;

		enable_cv_.wait(lock, [this]{ return exec_calls_count_ == 0; });

		logsys::log(
			[this](logsys::stdlogb& os){
				os << "chain(" << name << ") disabled";
			},
			[this]{
				// disable all modules
				for(std::size_t i = 0; i < modules_.size(); ++i){
					logsys::log([this, i](logsys::stdlogb& os){
							os << "chain(" << name << ") module("
								<< modules_[i]->number << ":"
								<< modules_[i]->type_name << ") disabled";
						}, [this, i]{
							modules_[i]->disable(chain_key());
						});
				}
			});
	}


	template < typename F >
	void chain::process_module(
		std::size_t const i,
		std::size_t const run,
		F const& action,
		std::string_view action_name
	){
		// lock mutex and wait for the previous run to be ready
		std::unique_lock< std::mutex > lock(module_mutexes_[i]);
		module_cv_.wait(lock, [this, i, run]{ return ready_run_[i] == run; });

		// exec or cleanup the module
		logsys::log([this, i, action_name](logsys::stdlogb& os){
			os << "id(" << modules_[i]->id << ") "
				<< "chain(" << modules_[i]->chain << ") module("
				<< modules_[i]->number << ":" << modules_[i]->type_name
				<< ") " << action_name;
		}, [i, &action]{ action(i); });

		// make module ready
		ready_run_[i] = run + 1;
		module_cv_.notify_all();
	}



}
