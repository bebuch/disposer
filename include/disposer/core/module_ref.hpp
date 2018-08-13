//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_ref__hpp_INCLUDED_
#define _disposer__core__module_ref__hpp_INCLUDED_

#include "optional_component.hpp"
#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"
#include "exec_module.hpp"

#include "../tool/false_c.hpp"

#include <logsys/log_ref.hpp>


namespace disposer{


	/// \brief Ref of a module during exec calls
	template <
		typename TypeList,
		typename State,
		typename ExecInputs,
		typename ExecOutputs,
		typename Parameters,
		typename Component >
	class module_ref
		: public optional_component< Component >
		, public logsys::log_ref
	{
	public:
		/// \brief Constructor
		template <
			typename Inputs,
			typename Outputs,
			typename ModuleInitFn,
			typename ExecFn,
			bool CanRunConcurrent >
		module_ref(
			exec_module< TypeList, Inputs, Outputs, Parameters,
				ModuleInitFn, ExecFn, CanRunConcurrent, Component >& module,
			State* state
		)noexcept
			: optional_component< Component >(module.component())
			, logsys::log_ref(module.location())
			, module_(module)
			, state_(state)
			, inputs_(module.inputs())
			, outputs_(module.outputs())
			, parameters_(module.parameters()) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return get(*this, name);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return get(*this, name);
		}

		/// \brief Get type by dimension index
		template < std::size_t DI >
		static constexpr auto dimension(hana::size_t< DI > i)noexcept{
			static_assert(DI < TypeList::type_count,
				"module has less then DI dimensions");
			return TypeList::types[i];
		}

		/// \brief Get access to the state object if one exists
		auto& state()noexcept{
			static_assert(!std::is_void_v< State >, "Module has no state.");
			return *state_;
		}


		/// \brief Disposer global ID of this exec
		std::size_t id()const noexcept{
			return module_.id();
		}

		/// \brief Chain local ID of this exec
		std::size_t exec_id()const noexcept{
			return module_.exec_id();
		}


		/// \brief Location for log messages
		std::string_view location()const noexcept{
			return module_.location();
		}


		/// \brief Name of the process chain in config file section 'chain'
		std::string_view chain()const noexcept{
			return module_.chain();
		}

		/// \brief Name of the module type given via class declarant
		std::string_view type_name()const noexcept{
			return module_.type_name();
		}

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 1.
		std::size_t number()const noexcept{
			return module_.number();
		}


	private:
		template < typename This, typename Name >
		static auto& get(This& this_, Name const& name)noexcept{
			using name_t = std::remove_reference_t< Name >;
			if constexpr(hana::is_a< input_name_tag, name_t >()){
				auto const index = hana::index_if(this_.inputs_,
					[name](auto const& p){ return p.name == name; });

				static_assert(!hana::is_nothing(index),
					"input name doesn't exist");

				return this_.inputs_[*index];
			}else if constexpr(hana::is_a< output_name_tag, name_t >()){
				auto const index = hana::index_if(this_.outputs_,
					[name](auto const& p){ return p.name == name; });

				static_assert(!hana::is_nothing(index),
					"output name doesn't exist");

				return this_.outputs_[*index];
			}else if constexpr(hana::is_a< parameter_name_tag, name_t >()){
				auto const index = hana::index_if(this_.parameters_,
					[name](auto const& p){ return p.name == name; });

				static_assert(!hana::is_nothing(index),
					"parameter name doesn't exist");

				return this_.parameters_[*index].get();
			}else{
				static_assert(detail::false_c< Name >,
					"name is not an input_name, output_name or parameter_name");
			}
		}


		/// \brief Reference to the exec_module
		exec_module_base& module_;

		/// \brief Module state
		///
		/// nullptr-pointer to void if module is stateless.
		State* state_;

		/// \brief hana::tuple of exec_inputs
		ExecInputs& inputs_;

		/// \brief hana::tuple of exec_outputs
		ExecOutputs& outputs_;

		/// \brief hana::tuple of parameters
		Parameters const& parameters_;
	};


	template <
		typename TypeList,
		typename State,
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	module_ref(
		exec_module< TypeList, Inputs, Outputs, Parameters,
			ModuleInitFn, ExecFn, CanRunConcurrent, Component >& module,
		State* state
	) -> module_ref<
		TypeList,
		State,
		to_exec_list_t< Inputs >,
		to_exec_list_t< Outputs >,
		Parameters,
		Component >;


}


#endif
