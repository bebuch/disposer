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

#include "exec_input.hpp"
#include "exec_output.hpp"
#include "parameter.hpp"


namespace disposer{


	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename StateMakerFn,
		typename ExecFn >
	class module;


	template < typename IOList >
	struct to_exec_list;

	template < typename ... Names, typename ... Ts >
	struct to_exec_list< hana::tuple< output< Names, Ts > ... > >{
		using type = hana::tuple< exec_output< Names, Ts > ... >;
	};

	template < typename ... Names, typename ... Ts, bool ... IsRequireds >
	struct to_exec_list< hana::tuple< input< Names, Ts, IsRequireds > ... > >{
		using type = hana::tuple< exec_input< Names, Ts, IsRequireds > ... >;
	};

	template < typename IOList >
	using to_exec_list_t = typename to_exec_list< IOList >::type;


	/// \brief The actual exec_module type
	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename StateMakerFn,
		typename ExecFn >
	class exec_module: public exec_module_base{
	public:
		/// \brief Constructor
		exec_module(
			module< Inputs, Outputs, Parameters, StateMakerFn, ExecFn >& module,
			std::size_t id,
			output_map_type& output_map
		)noexcept
			: module_(module)
			, id_(id)
			, inputs_(module.inputs(output_map))
			, outputs_(module.outputs(output_map))
			, location_("id(" + std::to_string(id_) + ") chain("
				+ module_.chain() + ") module("
				+ std::to_string(module_.number()) + ":"
				+ module_.type_name() + ") exec: ") {}


	private:
		/// \brief Reference to the module
		module< List, StateMakerFn, ExecFn >& module_;

		/// \brief Current exec id
		std::size_t const id_;

		/// \brief hana::tuple of exec_inputs
		to_exec_list_t< Inputs > inputs_;

		/// \brief hana::tuple of exec_outputs
		to_exec_list_t< Outputs > outputs_;

		/// \brief Location for log messages
		std::string const location_;


		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			module_.exec(id_, inputs_, outputs_, location_);
		}

		/// \brief Cleanup inputs and connected outputs if appropriate
		virtual void cleanup()noexcept override{
			hana::for_each(inputs_, [](auto& input){ input.cleanup(); });
		}
	};


}


#endif
