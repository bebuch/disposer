//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/module_base.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <numeric>


namespace disposer{


	chain::chain(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain,
		id_generator& generate_id
	):
		name(config_chain.name),
		modules_(create_chain_modules(maker_list, config_chain)),
		generate_id_(generate_id),
		next_run_(0),
		ready_run_(modules_.modules.size()),
		module_mutexes_(modules_.modules.size()),
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


	std::vector< exec_module_ptr >
	chain::make_exec_modules(std::size_t const id){
		std::vector< exec_module_ptr > list;
		list.reserve(modules_.modules.size());

		output_map_type output_map;
		for(auto const& data: modules_.modules){
			list.push_back(data.module->make_exec_module(id, output_map));
		}

		return list;
	}


	void chain::exec(){
		if(!enabled_){
			throw std::logic_error("chain(" + name + ") is not enabled");
		}

		exec_call_manager lock(exec_calls_count_, enable_cv_);

		// generate a new id for the exec
		std::size_t const id = generate_id_();

		// exec any module, call cleanup instead if the module throw
		logsys::log([this, id](logsys::stdlogb& os){
			os << "id(" << id << ") chain(" << name << ")";
		}, [this, id]{
			auto modules = logsys::log([this, id](logsys::stdlogb& os){
					os << "id(" << id << ") chain(" << name << ") prepared";
				}, [this, id]{
					return make_exec_modules(id);
				});

			try{
				for(std::size_t i = 0; i < modules.size(); ++i){
					process_module(i, id, [&modules](std::size_t i){
						modules[i]->exec();
						modules[i]->cleanup();
					}, "exec");
				}
			}catch(...){
				// cleanup and unlock all executions
				for(std::size_t i = 0; i < ready_run_.size(); ++i){
					// exec was successful
					if(ready_run_[i] >= id + 1) continue;

					process_module(i, id, [&modules](std::size_t i){
						modules[i]->cleanup();
					}, "cleanup");
				}

				// rethrow exception
				throw;
			}
		});
	}


	void chain::enable(){
		// TODO: Prevent enable while disable is waiting on exec's
		std::unique_lock< std::mutex > lock(enable_mutex_);
		if(enabled_) return;

		// TODO: This line is redundant, right?
		enable_cv_.wait(lock, [this]{ return exec_calls_count_ == 0; });

		logsys::log(
			[this](logsys::stdlogb& os){
				os << "chain(" << name << ") enabled";
			},
			[this]{
				std::size_t i = 0;
				try{
					// enable all modules
					for(; i < modules_.modules.size(); ++i){
						auto& module = modules_.modules[i].module;
						logsys::log([this, &module](logsys::stdlogb& os){
								os << "chain(" << name << ") module("
									<< module->number << ":"
									<< module->type_name
									<< ") enabled";
							}, [&module]{
								module->enable();
							});
					}
				}catch(...){
					// disable all modules until the one who throw
					for(std::size_t j = 0; j < i; ++j){
						auto& module = modules_.modules[j].module;
						logsys::log([this, i, &module](logsys::stdlogb& os){
								os << "chain(" << name << ") module("
									<< module->number
									<< ") disabled because of exception while "
									<< "enable module("
									<< modules_.modules[i].module->number
									<< ")";
							}, [&module]{
								module->disable();
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
				for(std::size_t i = 0; i < modules_.modules.size(); ++i){
					auto& module = modules_.modules[i].module;
					logsys::log([this, &module](logsys::stdlogb& os){
							os << "chain(" << name << ") module("
								<< module->number << ":"
								<< module->type_name << ") disabled";
						}, [&module]{
							module->disable();
						});
				}
			});
	}


	template < typename F >
	void chain::process_module(
		std::size_t const i,
		std::size_t const id,
		F const& action,
		std::string_view action_name
	){
		// lock mutex and wait for the previous run to be ready
		std::unique_lock< std::mutex > lock(module_mutexes_[i]);
		module_cv_.wait(lock, [this, i, id]{ return ready_run_[i] == id; });

		// exec or cleanup the module
		logsys::log([this, id, i, action_name](logsys::stdlogb& os){
			auto& module = modules_.modules[i].module;
			os << "id(" << id << ") "
				<< "chain(" << module->chain << ") module(" << module->number
				<< ":" << module->type_name << ") " << action_name;
		}, [i, &action]{ action(i); });

		// make module ready
		ready_run_[i] = id + 1;
		module_cv_.notify_all();
	}



}
