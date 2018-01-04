//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/chain.hpp>
#include <disposer/core/module_base.hpp>

#include <disposer/config/create_chain_modules.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <numeric>
#include <future>


namespace disposer{


	chain::chain(
		module_maker_list const& maker_list,
		types::embedded_config::chain const& config_chain,
		id_generator& generate_id
	)
		: name(config_chain.name)
		, modules_(create_chain_modules(maker_list, config_chain))
		, generate_id_(generate_id)
		, enable_count_(0)
		, locked_(false)
		, exec_calls_count_(0) {}


	chain::~chain(){
		assert(enable_count_ == 0);
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


		/// \brief A module and its execution data
		class chain_exec_module_data{
		public:
			chain_exec_module_data(
				chain_exec_module_data* data,
				chain_module_data const& module_data,
				exec_module_ptr&& exec_module
			)
				: module(std::move(exec_module))
				, precursor_count(module_data.precursor_count)
				, precursor_failed(false)
				, next_module([data, &module_data]{
						std::vector< chain_exec_module_data* > init;
						init.reserve(module_data.next_indexes.size());
						for(std::size_t const i: module_data.next_indexes){
							init.push_back(data + i);
						}
						return init;
					}())
			{
				exec_list.reserve(next_module.size());
			}

			/// \brief Move constructor
			///
			/// This is a hack …
			///
			/// TODO: Implement chain_exec_module_list so that this constructor
			///       is not needed.
			chain_exec_module_data(chain_exec_module_data&& other)noexcept
				: module(std::move(other.module))
				, precursor_count(other.precursor_count.load())
				, precursor_failed(other.precursor_failed.load())
				, next_module(std::move(other.next_module))
				, exec_list(std::move(other.exec_list)) {}

			bool exec_next(bool const precurser_succeeded)noexcept{
				if(precurser_succeeded){
					if(--precursor_count == 0) return exec(true);
				}else{
					if(!precursor_failed.exchange(true)) exec(false);
				}
				return precurser_succeeded;
			}

			bool exec(bool const precurser_succeeded)noexcept{
				auto success = precurser_succeeded;

				if(precurser_succeeded){
					success = module->exec();
				}

				module->cleanup();

				std::transform(next_module.begin(), next_module.end(),
					std::back_inserter(exec_list),
					[success](chain_exec_module_data* ptr){
						return std::async([success, ptr]{
							return ptr->exec_next(success);
						});
					});
				for(auto& exec: exec_list){
					auto success_exec = exec.get();
					success = success_exec && success;
				}
				return success;
			}

		private:
			/// \brief The module
			exec_module_ptr module;

			/// \brief The count of modules that must be ready before execution
			std::atomic< std::size_t > precursor_count;

			/// \brief The count of modules that must be ready before execution
			std::atomic< std::size_t > precursor_failed;

			/// \brief Pointers to all modules that depend on this module
			std::vector< chain_exec_module_data* > next_module;

			/// \brief Memory for the async executions of the next modules
			///
			/// This is a member since the memory allocation might throw. To
			/// make exec() noexcept, the memory is allocated in the
			/// constructor by reserve().
			std::vector< std::future< bool > > exec_list;
		};


		/// \brief List of a chains modules and indexes of the start modules
		class chain_exec_module_list{
		public:
			chain_exec_module_list(
				chain_module_list const& module_list,
				std::vector< exec_module_ptr >&& list
			)
				: modules([&]{
						std::vector< chain_exec_module_data > init;
						init.reserve(list.size());
						std::transform(
							module_list.modules.begin(),
							module_list.modules.end(),
							std::make_move_iterator(list.begin()),
							std::back_inserter(init),
							[data = init.data()](
								chain_module_data const& module_data,
								exec_module_ptr&& exec_module
							){
								return chain_exec_module_data(data,
									module_data, std::move(exec_module));
							});
						return init;
					}())
				, start_modules([this, &module_list]{
						std::vector< chain_exec_module_data* > init;
						init.reserve(module_list.start_indexes.size());
						for(std::size_t const i: module_list.start_indexes){
							init.push_back(modules.data() + i);
						}
						return init;
					}())
			{
				exec_list.reserve(start_modules.size());
			}

			bool exec()noexcept{
				std::transform(start_modules.begin(), start_modules.end(),
					std::back_inserter(exec_list),
					[](chain_exec_module_data* ptr){
						return std::async([ptr]{
							return ptr->exec(true);
						});
					});

				auto success = true;
				for(auto& exec: exec_list){
					success = exec.get() && success;
				}

				return success;
			}

		private:
			/// \brief List of modules and there execution data
			std::vector< chain_exec_module_data > modules;

			/// \brief Pointers to all modules without active inputs
			std::vector< chain_exec_module_data* > const start_modules;

			/// \brief Memory for the async executions of the starter modules
			///
			/// This is a member since the memory allocation might throw. To
			/// make exec() noexcept, the memory is allocated in the
			/// constructor by reserve().
			std::vector< std::future< bool > > exec_list;
		};


		chain_exec_module_list make_exec_modules(
			chain_module_list const& module_list,
			std::size_t const id,
			std::size_t const exec_id
		){
			std::vector< exec_module_ptr > list;
			list.reserve(module_list.modules.size());

			output_map_type output_map;
			for(auto const& data: module_list.modules){
				list.push_back(data.module
					->make_exec_module(id, exec_id, output_map));
			}

			return chain_exec_module_list(module_list, std::move(list));
		}


	}


	bool chain::exec(){
		if(enable_count_ == 0){
			throw std::logic_error("chain(" + name + ") is not enabled");
		}

		exec_call_manager lock(exec_calls_count_, enable_cv_);

		// generate a new id for the exec
		std::size_t const id = generate_id_();
		std::size_t const exec_id = generate_exec_id_();

		// exec any module, call cleanup instead if the module throw
		return logsys::log([this, id](logsys::stdlogb& os){
			os << "id(" << id << ") chain(" << name << ")";
		}, [this, id, exec_id]{
			auto modules = logsys::log([this, id](logsys::stdlogb& os){
					os << "id(" << id << ") chain(" << name << ") prepared";
				}, [this, id, exec_id]{
					return make_exec_modules(modules_, id, exec_id);
				});

			return modules.exec();
		});
	}


	void chain::enable(){
		std::unique_lock< std::mutex > lock(enable_mutex_);

		if(locked_){
			throw std::runtime_error("can not enable chain(" + name
				+ ") while it is locked");
		}

		if(enable_count_ == 0){
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
										<< ") disabled because of exception "
										<< "while enable module("
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
		}

		++enable_count_;
	}

	void chain::disable()noexcept{
		std::unique_lock< std::mutex > lock(enable_mutex_);

		assert(enable_count_ > 0);

		if(--enable_count_ == 0){
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
	}


	void chain::lock(){
		std::unique_lock< std::mutex > lock(enable_mutex_);

		if(enable_count_ > 0){
			throw chain_not_lockable(name);
		}

		assert(!locked_);

		locked_ = true;
	}

	void chain::unlock(){
		std::unique_lock< std::mutex > lock(enable_mutex_);

		assert(locked_);

		locked_ = false;
	}


}
