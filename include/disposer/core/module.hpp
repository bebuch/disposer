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
#include "module_exec.hpp"
#include "state_maker_fn.hpp"
#include "exec_fn.hpp"

#include "../tool/exec_list_t.hpp"

#include <cassert>


namespace disposer{


	/// \brief The actual module type
	template < typename List, typename StateMakerFn, typename ExecFn >
	class module: public module_base{
	public:
		/// \brief State maker function or void for stateless modules
		using state_maker_fn_type = StateMakerFn;


		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		module(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			state_maker_fn< StateMakerFn > const& state_maker_fn,
			exec_fn< ExecFn > const& exec_fn
		)
			: module_base(data.chain, data.type_name, data.number)
			, data_(maker_list, data, location, std::make_index_sequence<
				decltype(hana::size(maker_list))::value >())
			, state_(state_maker_fn)
			, exec_fn_(exec_fn) {}


		/// \brief Name of the process chain in config file section 'chain'
		std::string const& chain()const noexcept{
			return module_base::chain;
		}

		/// \brief Name of the module type given via class module_declarant
		std::string const& type_name()const noexcept{
			return module_base::type_name;
		}

		/// \brief Position of the module in the process chain
		std::size_t number()const noexcept{
			return module_base::number;
		}


		/// \brief Calls the exec_fn
		void exec(
			std::size_t id,
			module_exec_data< detail::exec_list_t< List > >& exec_data
		)const{
			return exec_fn_(accessory(id, exec_data, data_, state_));
		}


	private:
		/// \brief Enables the module for exec calls
		///
		/// Build a users state object.
		virtual void enable()override{
			state_.enable(
				static_cast< module_data< List > const& >(data_),
				this->location);
		}

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept override{
			state_.disable();
		}


		/// \brief Make a corresponding module_exec
		virtual module_exec_ptr make_module_exec(
			std::size_t id, output_map_type& output_map
		){
			return std::make_unique< module_exec< List, StateMakerFn, ExecFn > >
				(*this, id, output_map);
		}


		/// \brief Get map from output names to output_base pointers
		virtual output_name_to_ptr_type output_name_to_ptr()override{
			return data_.output_name_to_ptr();
		}


		/// \brief Module config file data
		module_data< List > data_;

		/// \brief The user defined state object
		state< List, StateMakerFn > state_;

		/// \brief The function called on exec
		exec_fn< ExecFn > exec_fn_;
	};


}


#endif
