//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_exec__hpp_INCLUDED_
#define _disposer__core__module_exec__hpp_INCLUDED_

#include "module_exec_base.hpp"
#include "state_maker_fn.hpp"
#include "exec_fn.hpp"

#include <cassert>


namespace disposer{


	/// \brief The actual module_exec type
	template < typename List, typename StateMakerFn, typename ExecFn >
	class module_exec: public module_base{
	public:
		/// \brief Type for state_maker_fn
		using state_accessory_type = state_accessory< List >;

		/// \brief Type for exec_fn
		using exec_accessory_type = exec_accessory< List >;


		/// \brief Type of the module_exec state object
		using state_type = std::invoke_result_t<
			module_state_maker< StateMakerFn >, state_accessory_type const& >;

		static_assert(!std::is_void_v< state_type >,
			"state_maker function must not return void");


		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		module_exec(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			module_state_maker< StateMakerFn > const& state_maker_fn,
			module_exec< ExecFn > const& exec_fn
		)
			: module_base(data.type_name, data.chain, data.number,
				// data_ can be referenced before initialization
				generate_input_list(data_),
				generate_output_list(data_))
			, data_(maker_list, data, location, std::make_index_sequence<
				decltype(hana::size(maker_list))::value >())
			, state_maker_fn_(state_maker_fn)
			, exec_fn_(exec_fn) {}


		/// \brief Modules are not copyable
		module_exec(module_exec const&) = delete;

		/// \brief Modules are not movable
		module_exec(module_exec&&) = delete;



	private:
		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			assert(state_);
			exec_fn_(accessory_, id, *state_);
		}


		/// \brief Module config file data
		module_data< List > data_;

		/// \brief The function object that is called in enable()
		module_state_maker< StateMakerFn > state_maker_fn_;

		/// \brief The function object that is called in exec()
		std::optional< state_type > state_;
	};


}


#endif
