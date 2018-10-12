//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_module__hpp_INCLUDED_
#define _disposer__core__exec_module__hpp_INCLUDED_

#include "exec_module_base.hpp"
#include "optional_component.hpp"
#include "to_exec_list.hpp"
#include "parameter.hpp"

#include <boost/hana/for_each.hpp>


namespace disposer{


	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	class module;

	/// \brief The actual exec_module type
	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	class exec_module: public exec_module_base{
		/// \brief Type of the module
		using module_type = class module< TypeList, Inputs, Outputs, Parameters,
			ModuleInitFn, ExecFn, CanRunConcurrent, Component >;

	public:
		/// \brief Constructor
		exec_module(
			module_type& module,
			to_exec_init_list_t< Inputs >&& inputs,
			to_exec_init_list_t< Outputs >&& outputs,
			std::size_t id,
			std::size_t exec_id
		)noexcept
			: exec_module_base(module, id, exec_id)
			, inputs_(std::move(inputs))
			, outputs_(std::move(outputs)) {}


		/// \brief hana::tuple of exec_inputs
		to_exec_list_t< Inputs >& inputs(){
			return inputs_;
		}

		/// \brief hana::tuple of exec_outputs
		to_exec_list_t< Outputs >& outputs(){
			return outputs_;
		}

		/// \brief hana::tuple of parameters
		Parameters const& parameters(){
			return module().parameters();
		}


		/// \brief Reference to component or empty struct
		optional_component< Component >& component(){
			return module().component();
		}


	private:
		/// \brief Reference to the original module
		module_type& module(){
			return static_cast< module_type& >(module_);
		}

		/// \brief Const reference to the original module
		module_type const& module()const{
			return static_cast< module_type const& >(module_);
		}


		/// \brief hana::tuple of exec_inputs
		to_exec_list_t< Inputs > inputs_;

		/// \brief hana::tuple of exec_outputs
		to_exec_list_t< Outputs > outputs_;


		/// \brief The actual worker function called one times per trigger
		virtual bool exec()noexcept override{
			return module().exec(*this);
		}

		/// \brief Cleanup inputs and connected outputs if appropriate
		virtual void cleanup()noexcept override{
			hana::for_each(inputs_, [](auto& input){ input.cleanup(); });
		}
	};


}


#endif
