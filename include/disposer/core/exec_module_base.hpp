//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_module_base__hpp_INCLUDED_
#define _disposer__core__exec_module_base__hpp_INCLUDED_

#include "exec_input_base.hpp"


namespace disposer{


	/// \brief Base class for module exec object
	class exec_module_base{
	public:
		/// \brief Type of a map from output names to exec_output_base pointers
		using output_map = std::map< std::string_view, exec_output_base* >;


		/// \brief Constructor
		exec_module_base()noexcept = default;

		/// \brief Modules are not copyable
		exec_module_base(exec_module_base const&) = delete;

		/// \brief Modules are not movable
		exec_module_base(exec_module_base&&) = delete;


		/// \brief Modules are not copyable
		exec_module_base& operator=(exec_module_base const&) = delete;

		/// \brief Modules are not movable
		exec_module_base& operator=(exec_module_base&&) = delete;


		/// \brief Standard virtual destructor
		virtual ~exec_module_base() = default;


		/// \brief The worker function
		virtual bool exec()noexcept = 0;

		/// \brief Called for every module after a successfull exec or after
		///        a throwing exec on the module and all following modules
		///        in the chain without previos exec call
		virtual void cleanup()noexcept = 0;
	};


}


#endif
