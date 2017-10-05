//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/module_base.hpp>

#include <disposer/config/create_chain_modules.hpp>

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
				, next_module([data, &module_data]{
						std::vector< chain_exec_module_data* > init;
						init.reserve(module_data.next_indexes.size());
						std::transform(
							module_data.next_indexes.begin(),
							module_data.next_indexes.end(),
							std::back_inserter(init),
							[data](std::size_t i){ return data + 1; });
						return init;
					}()) {}

			void exec_next(){
				if(--precursor_count == 0) exec();
			}

			void exec(){
				try{
					module->exec();
					module->cleanup();
				}catch(...){
					module->cleanup();

					throw;
				}

				std::vector< std::future< void > > list;
				list.reserve(next_module.size());
				std::transform(next_module.begin(), next_module.end(),
					std::back_inserter(list), [](chain_exec_module_data* ptr){
						return std::async([ptr]{ ptr->exec_next(); });
					});
				for(auto& exec: list) exec.get();
			}

		private:
			/// \brief The module
			exec_module_ptr const module;

			/// \brief The count of modules that must be ready before execution
			std::atomic< std::size_t > precursor_count;

			/// \brief Pointers to all modules that depend on this module
			std::vector< chain_exec_module_data* > const next_module;
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
						auto data = init.data();
						std::transform(
							module_list.modules.begin(),
							module_list.modules.end(),
							std::make_move_iterator(list.begin()),
							std::back_inserter(init),
							[data](
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
						auto data = modules.data();
						std::transform(
							module_list.start_indexes.begin(),
							module_list.start_indexes.end(),
							std::back_inserter(init),
							[&data](std::size_t i){ return data + i; });
						return init;
					}()) {}

			void exec(){
				std::vector< std::future< void > > list;
				list.reserve(start_modules.size());
				std::transform(start_modules.begin(), start_modules.end(),
					std::back_inserter(list), [](chain_exec_module_data* ptr){
						return std::async([ptr]{ ptr->exec(); });
					});
				for(auto& exec: list) exec.get();
			}

		private:
			/// \brief List of modules and there execution data
			std::vector< chain_exec_module_data > modules;

			/// \brief Pointers to all modules without active inputs
			std::vector< chain_exec_module_data* > const start_modules;
		};


		chain_exec_module_list make_exec_modules(
			chain_module_list const& module_list,
			std::size_t const id
		){
			std::vector< exec_module_ptr > list;
			list.reserve(module_list.modules.size());

			output_map_type output_map;
			for(auto const& data: module_list.modules){
				list.push_back(data.module->make_exec_module(id, output_map));
			}

			return chain_exec_module_list(module_list, std::move(list));
		}


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
					return make_exec_modules(modules_, id);
				});

			modules.exec();

// 			std::size_t i = 0;
// 			try{
// 				for(; i < modules.modules.size(); ++i){
// 					process_module(i, id, [&modules](std::size_t i){
// 						modules.modules[i].module->exec();
// 						modules.modules[i].module->cleanup();
// 					}, "exec");
// 				}
// 			}catch(...){
// 				// cleanup
// 				for(; i < modules.modules.size(); ++i){
// 					process_module(i, id, [&modules](std::size_t i){
// 						modules.modules[i].module->cleanup();
// 					}, "cleanup");
// 				}
//
// 				// rethrow exception
// 				throw;
// 			}
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
		// exec or cleanup the module
		logsys::log([this, id, i, action_name](logsys::stdlogb& os){
			auto& module = modules_.modules[i].module;
			os << "id(" << id << ") "
				<< "chain(" << module->chain << ") module(" << module->number
				<< ":" << module->type_name << ") " << action_name;
		}, [i, &action]{ action(i); });
	}



}
