//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module__hpp_INCLUDED_
#define _disposer__core__module__hpp_INCLUDED_

#include "output_name_to_ptr_type.hpp"
#include "module_base.hpp"
#include "exec_module.hpp"
#include "module_init_fn.hpp"
#include "exec_fn.hpp"

#include <mutex>
#include <condition_variable>


namespace disposer{


	template < bool CanRunConcurrent >
	class concurrency_manager{
	public:
		constexpr void wait(std::size_t const)noexcept{}
		constexpr void ready(std::size_t const)noexcept{}
	};

	template <>
	class concurrency_manager< false >{
	public:
		concurrency_manager()noexcept: next_exec_id_(0) {}

		void wait(std::size_t const exec_id)noexcept{
			std::unique_lock lock(mutex_);
			cv_.wait(lock,
				[this, exec_id]{ return next_exec_id_ == exec_id; });
		}

		void ready(std::size_t const exec_id)noexcept{
			std::unique_lock lock(mutex_);
			next_exec_id_ = exec_id + 1;
			lock.unlock();
			cv_.notify_all();
		}

	private:
		std::size_t next_exec_id_;
		std::mutex mutex_;
		std::condition_variable cv_;
	};

	template < typename Manager >
	class concurrency_manager_guard{
	public:
		concurrency_manager_guard(Manager& manager, std::size_t exec_id)noexcept
			: manager_(manager)
			, exec_id_(exec_id)
		{
			static_assert(noexcept(manager_.wait(exec_id_)));
			manager_.wait(exec_id_);
		}

		~concurrency_manager_guard(){
			static_assert(noexcept(manager_.ready(exec_id_)));
			manager_.ready(exec_id_);
		}

	private:
		Manager& manager_;
		std::size_t exec_id_;
	};


	template < typename ModuleInitFn, typename ModuleInitRef >
	struct module_init_fn_result{
		using type = std::invoke_result_t<
			module_init_fn< ModuleInitFn >, ModuleInitRef >;
	};

	template < typename ModuleInitRef >
	struct module_init_fn_result< void, ModuleInitRef >{
		using type = void;
	};

	template < typename ModuleInitFn, typename ModuleInitRef >
	using module_init_fn_result_t =
		typename module_init_fn_result< ModuleInitFn, ModuleInitRef >::type;


	/// \brief The actual module type
	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	class module
		: public concurrency_manager< CanRunConcurrent >
		, public module_base
	{
		/// \brief State maker function or void for stateless modules
		using module_init_fn_type = ModuleInitFn;

		/// \brief Type of the module state object
		using state_type = module_init_fn_result_t<
			ModuleInitFn, module_init_ref<
				TypeList, Inputs, Outputs, Parameters, Component > >;

		/// \brief Type of the module data
		using module_data_type =
			module_data< TypeList, Inputs, Outputs, Parameters >;

		/// \brief Type of the user defined state object
		using module_state_type = module_state< state_type, ModuleInitFn >;

		/// \brief Type of exec_module
		using exec_module_type = exec_module< TypeList, Inputs, Outputs,
			Parameters, ModuleInitFn, ExecFn, CanRunConcurrent,
			Component >;


	public:
		/// \brief Constructor
		template < typename ... Ts, typename ... RefList >
		module(
			type_list< Ts ... >,
			std::string const& chain,
			std::string const& type_name,
			std::size_t number,
			hana::tuple< RefList ... >&& ref_list,
			module_init_fn< ModuleInitFn > const& module_init_fn,
			exec_fn< ExecFn > const& exec_fn,
			hana::bool_< CanRunConcurrent >,
			optional_component< Component > component
		)
			: module_base(chain, type_name, number)
			, data_(std::move(ref_list))
			, state_(module_init_fn)
			, component_(component)
			, exec_fn_(exec_fn) {
				if constexpr(!std::is_void_v< Component >){
					++component_.usage_count;
				}
			}

		~module(){
			if constexpr(!std::is_void_v< Component >){
				--component_.usage_count;
			}
		}


		/// \brief Calls the exec_fn
		bool exec(exec_module_type& exec_module)noexcept{
			auto const id = exec_module.id();
			module_ref ref{exec_module, state_.object()};
			concurrency_manager_guard< concurrency_manager< CanRunConcurrent > >
				manager(*this, exec_module.exec_id());
			return logsys::exception_catching_log(
				[this, id](logsys::stdlogb& os){
					os << "id(" << id << ") " << this->log_prefix() << "exec";
				}, [this, &ref]{ exec_fn_(ref); });
		}


		/// \brief Enables the module for exec calls
		///
		/// Build a users state object.
		virtual void enable()override{
			state_.enable(static_cast< module_data_type const& >(data_),
				component_, this->log_prefix());
		}

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept override{
			state_.disable();
		}


		/// \brief Make a corresponding exec_module
		virtual exec_module_ptr make_exec_module(
			std::size_t const id,
			std::size_t const exec_id,
			output_map_type& output_map
		)override{
			return std::make_unique< exec_module_type >(*this,
					hana::transform(data_.inputs,
						[&output_map](auto const& input){
							return hana::tuple
								< decltype(input), output_map_type const& >
								{input, output_map};
						}),
					hana::transform(data_.outputs,
						[this, id, &output_map](auto& output){
							return exec_output_init_data(
								output, output_map, id, this->log_prefix());
						}), id, exec_id
				);
		}


		/// \brief Get map from output names to output_base pointers
		virtual output_name_to_ptr_type output_name_to_ptr()override{
			return hana::unpack(data_.outputs, [](auto& ... output){
					return output_name_to_ptr_type{
							{detail::to_std_string(output.name), &output} ...
						};
				});
		}


		/// \brief hana::tuple of parameters
		Parameters const& parameters()const{
			return data_.parameters;
		}


		/// \brief Reference to component or empty struct
		optional_component< Component >& component(){
			return component_;
		}


	private:
		/// \brief inputs, outputs and parameters
		module_data_type data_;

		/// \brief The user defined state object
		module_state_type state_;

		/// \brief Reference to component or empty struct
		optional_component< Component > component_;

		/// \brief The function called on exec
		exec_fn< ExecFn > exec_fn_;
	};

	template <
		typename ... Ts,
		typename ... RefList,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	module(
		type_list< Ts ... >,
		std::string const& chain,
		std::string const& type_name,
		std::size_t number,
		hana::tuple< RefList ... >&& ref_list,
		module_init_fn< ModuleInitFn > const& module_init_fn,
		exec_fn< ExecFn > const& exec_fn,
		hana::bool_< CanRunConcurrent >,
		optional_component< Component >
	)
		-> module<
			type_list< Ts ... >,
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< input_tag >)),
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< output_tag >)),
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< parameter_tag >)),
			ModuleInitFn, ExecFn, CanRunConcurrent, Component >;



}


#endif
