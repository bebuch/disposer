//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module__hpp_INCLUDED_
#define _disposer__core__module__hpp_INCLUDED_

#include "module_base.hpp"
#include "module_data.hpp"
#include "exec_module.hpp"
#include "to_exec_list.hpp"
#include "module_init_fn.hpp"
#include "exec_fn.hpp"


namespace disposer{


	using output_name_to_ptr_type
		= std::unordered_map< std::string, output_base* >;


	/// \brief The actual module type
	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn >
	class module: public module_base{
	public:
		/// \brief State maker function or void for stateless modules
		using module_init_fn_type = ModuleInitFn;

		/// \brief Type of the module state object
		using state_type = typename module_state<
			TypeList, Inputs, Outputs, Parameters, ModuleInitFn >::state_type;


		/// \brief Constructor
		template < typename ... Ts, typename ... RefList >
		module(
			type_list< Ts ... >,
			std::string const& chain,
			std::string const& type_name,
			std::size_t number,
			hana::tuple< RefList ... >&& ref_list,
			module_init_fn< ModuleInitFn > const& module_init_fn,
			exec_fn< ExecFn > const& exec_fn
		)
			: module_base(chain, type_name, number)
			, data_(std::move(ref_list))
			, state_(module_init_fn)
			, exec_fn_(exec_fn) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return data_(name);
		}


		/// \brief Calls the exec_fn
		void exec(
			std::size_t id,
			to_exec_list_t< Inputs >& inputs,
			to_exec_list_t< Outputs >& outputs,
			std::string_view location
		){
			module_accessory accessory{id, TypeList{}, state_.object(),
				inputs, outputs, data_.parameters, location};
			return exec_fn_(accessory);
		}


		/// \brief Enables the module for exec calls
		///
		/// Build a users state object.
		virtual void enable()override{
			state_.enable(static_cast< module_data< TypeList, Inputs, Outputs,
				Parameters > const& >(data_), this->location);
		}

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept override{
			state_.disable();
		}


		/// \brief Make a corresponding exec_module
		virtual exec_module_ptr make_exec_module(
			std::size_t id, output_map_type& output_map
		)override{
			return std::make_unique< exec_module< TypeList, Inputs, Outputs,
				Parameters, ModuleInitFn, ExecFn > >(*this,
					hana::transform(data_.inputs,
						[&output_map](auto const& input){
							return hana::tuple
								< decltype(input), output_map_type const& >
								{input, output_map};
						}),
					hana::transform(data_.outputs,
						[&output_map](auto& output){
							return hana::tuple
								< decltype(output), output_map_type& >
								{output, output_map};
						}), id
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


	private:
		/// \brief inputs, outputs and parameters
		module_data< TypeList, Inputs, Outputs, Parameters > data_;

		/// \brief The user defined state object
		module_state< TypeList, Inputs, Outputs, Parameters, ModuleInitFn >
			state_;

		/// \brief The function called on exec
		exec_fn< ExecFn > exec_fn_;
	};

	template <
		typename ... Ts,
		typename ... RefList,
		typename ModuleInitFn,
		typename ExecFn >
	module(
		type_list< Ts ... >,
		std::string const& chain,
		std::string const& type_name,
		std::size_t number,
		hana::tuple< RefList ... >&& ref_list,
		module_init_fn< ModuleInitFn > const& module_init_fn,
		exec_fn< ExecFn > const& exec_fn
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
			ModuleInitFn, ExecFn >;



}


#endif
