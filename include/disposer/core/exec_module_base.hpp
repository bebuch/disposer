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
#include "module_base.hpp"

#include <io_tools/make_string.hpp>


namespace disposer{


	/// \brief Base class for module exec object
	class exec_module_base{
	public:
		/// \brief Constructor
		exec_module_base(
			module_base& module,
			std::size_t id,
			std::size_t exec_id
		)noexcept
			: module_(module)
			, id_(id)
			, exec_id_(exec_id)
			, location_(io_tools::make_string(
				"id(", id, ") chain(", chain(), ") module(",
				number(), ":", type_name(), ") exec: ")) {}

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


		/// \brief Current id
		std::size_t id()const noexcept{
			return id_;
		}

		/// \brief Current exec id
		std::size_t exec_id()const noexcept{
			return exec_id_;
		}


		/// \brief Location for log messages
		std::string_view location()const noexcept{
			return location_;
		}


		/// \brief Name of the process chain in config file section 'chain'
		std::string_view chain()const noexcept{
			return module_.chain;
		}

		/// \brief Name of the module type given via class declarant
		std::string_view type_name()const noexcept{
			return module_.type_name;
		}

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 1.
		std::size_t number()const noexcept{
			return module_.number;
		}


	protected:
		/// \brief Reference to the module
		module_base& module_;


	private:
		/// \brief Current id
		///
		/// This id is unique over all chains.
		std::size_t id_;

		/// \brief Current exec id
		///
		/// This id is bound to the chain.
		std::size_t exec_id_;

		/// \brief Location for log messages
		std::string location_;
	};


}


#endif
