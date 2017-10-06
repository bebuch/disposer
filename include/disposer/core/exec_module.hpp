//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_module__hpp_INCLUDED_
#define _disposer__core__exec_module__hpp_INCLUDED_

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
		bool CanRunConcurrent >
	class module;

	/// \brief The actual exec_module type
	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent >
	class exec_module: public exec_module_base{
	public:
		/// \brief Constructor
		exec_module(
			module< TypeList, Inputs, Outputs, Parameters, ModuleInitFn,
				ExecFn, CanRunConcurrent >& module,
			to_exec_init_list_t< Inputs > const& inputs,
			to_exec_init_list_t< Outputs > const& outputs,
			std::size_t id
		)noexcept
			: module_(module)
			, id_(id)
			, inputs_(inputs)
			, outputs_(outputs)
			, location_("id(" + std::to_string(id_) + ") chain("
				+ module_.chain + ") module("
				+ std::to_string(module_.number) + ":"
				+ module_.type_name + ") exec: ") {}


	private:
		/// \brief Reference to the module
		module< TypeList, Inputs, Outputs, Parameters, ModuleInitFn, ExecFn,
			CanRunConcurrent >& module_;

		/// \brief Current exec id
		std::size_t const id_;

		/// \brief hana::tuple of exec_inputs
		to_exec_list_t< Inputs > inputs_;

		/// \brief hana::tuple of exec_outputs
		to_exec_list_t< Outputs > outputs_;

		/// \brief Location for log messages
		std::string const location_;


		/// \brief The actual worker function called one times per trigger
		virtual bool exec()noexcept override{
			return module_.exec(id_, inputs_, outputs_, location_);
		}

		/// \brief Cleanup inputs and connected outputs if appropriate
		virtual void cleanup()noexcept override{
			hana::for_each(inputs_, [](auto& input){ input.cleanup(); });
		}
	};


}


#endif
