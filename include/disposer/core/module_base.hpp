//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_base__hpp_INCLUDED_
#define _disposer__core__module_base__hpp_INCLUDED_

#include "output_name_to_ptr_type.hpp"
#include "output_map_type.hpp"
#include "output_base.hpp"
#include "input_base.hpp"

#include "../tool/module_ptr.hpp"


namespace disposer{


	/// \brief Base class for all modules
	class module_base{
	public:
		/// \brief Constructor
		module_base(
			std::string const& chain,
			std::string const& type_name,
			std::size_t number
		)
			: chain(chain)
			, type_name(type_name)
			, number(number)
			, location("chain(" + chain + ") module(" + std::to_string(number)
				+ ":" + type_name + "): "){}

		/// \brief Modules are not copyable
		module_base(module_base const&) = delete;

		/// \brief Modules are not movable
		module_base(module_base&&) = delete;


		/// \brief Modules are not copyable
		module_base& operator=(module_base const&) = delete;

		/// \brief Modules are not movable
		module_base& operator=(module_base&&) = delete;


		/// \brief Standard virtual destructor
		virtual ~module_base() = default;


		/// \brief Enables the module for exec calls
		virtual void enable() = 0;

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept = 0;


		/// \brief Make a corresponding exec_module
		virtual exec_module_ptr make_exec_module(
			std::size_t id,
			std::size_t exec_id,
			output_map_type& output_map) = 0;


		/// \brief Get map from output names to output_base pointers
		virtual output_name_to_ptr_type output_name_to_ptr() = 0;


		/// \brief Name of the process chain in config file section 'chain'
		std::string const chain;

		/// \brief Name of the module type given via class declarant
		std::string const type_name;

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 1.
		std::size_t const number;

		/// \brief Location text for log-messages
		std::string const location;
	};


}


#endif
