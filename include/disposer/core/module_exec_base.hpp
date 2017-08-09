//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_exec_base__hpp_INCLUDED_
#define _disposer__core__module_exec_base__hpp_INCLUDED_

#include "output_exec_base.hpp"
#include "input_exec_base.hpp"


namespace disposer{


	struct chain_key;


	/// \brief Base class for module exec object
	class module_exec_base{
	public:
		/// \brief Type of a map from output names to output_exec_base pointers
		using output_map = std::map< std::string_view, output_exec_base* >;


		/// \brief Constructor
		module_exec_base()noexcept = default;

		/// \brief Modules are not copyable
		module_exec_base(module_exec_base const&) = delete;

		/// \brief Modules are not movable
		module_exec_base(module_exec_base&&) = delete;


		/// \brief Modules are not copyable
		module_exec_base& operator=(module_exec_base const&) = delete;

		/// \brief Modules are not movable
		module_exec_base& operator=(module_exec_base&&) = delete;


		/// \brief Standard virtual destructor
		virtual ~module_exec_base() = default;


		/// \brief Call the actual worker function exec()
		void exec(chain_key&&){ exec(); }

		/// \brief Called for every module after a successfull exec or after
		///        a throwing exec on the module and all following modules
		///        in the chain without previos exec call
		void cleanup(chain_key&&)noexcept{ cleanup(); }


	protected:
		/// \brief The worker function
		virtual void exec() = 0;

		/// \brief The cleanup function
		virtual void cleanup()noexcept = 0;
	};


}


#endif
