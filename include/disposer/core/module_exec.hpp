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

#include "module_exec_data.hpp"

#include "../tool/exec_list_t.hpp"

#include <cassert>


namespace disposer{


	template < typename List, typename StateMakerFn, typename ExecFn >
	class module;

	/// \brief The actual module_exec type
	template < typename List, typename StateMakerFn, typename ExecFn >
	class module_exec: public module_exec_base{
	public:
		/// \brief Constructor
		module_exec(
			module< List, StateMakerFn, ExecFn >& module,
			std::size_t id,
			output_map_type& output_map
		)noexcept
			: module_(module)
			, id_(id)
			, data_(module, output_map, std::make_index_sequence<
				detail::input_output_count_c< List > >())
			, location_("id(" + std::to_string(id_) + ") chain("
				+ module_.chain() + ") module("
				+ std::to_string(module_.number()) + ":"
				+ module_.type_name() + ") exec: ") {}


	private:
		/// \brief Reference to the module
		module< List, StateMakerFn, ExecFn >& module_;

		/// \brief Current exec id
		std::size_t const id_;

		/// \brief List of input_exec's and output_exec's
		module_exec_data< detail::exec_list_t< List > > data_;

		/// \brief Location for log messages
		std::string const location_;


		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			module_.exec(id_, data_, location_);
		}

		/// \brief Cleanup inputs and connected outputs if appropriate
		virtual void cleanup()noexcept override{
			data_.cleanup();
		}
	};


}


#endif
