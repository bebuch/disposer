//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_base__hpp_INCLUDED_
#define _disposer__core__module_base__hpp_INCLUDED_

#include "module_exec_base.hpp"
#include "output_base.hpp"
#include "input_base.hpp"
#include "disposer.hpp"


namespace disposer{


	class module_base;

	/// \brief Class module_base access key
	struct module_base_key{
	private:
		/// \brief Constructor
		constexpr module_base_key()noexcept = default;
		friend class module_base;
	};


	struct chain_key;
	struct creator_key;


	using output_map_type
		= std::unordered_map< output_base*, output_exec_base* >;

	using output_name_to_ptr_type
		= std::unordered_map< std::string, output_base* >;


	/// \brief Base class for all disposer modules
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


		/// \brief Call the actual enable() function
		void enable(chain_key&&){ enable(); }

		/// \brief Call the actual disable() function
		void disable(chain_key&&)noexcept{ disable(); }

		/// \brief Call the actual make_module_exec(id) function
		module_exec_ptr make_module_exec(
			chain_key&&, std::size_t id, output_map_type& output_map
		){
			return make_module_exec(id, output_map);
		}


		/// \brief Call output_name_to_ptr()
		output_name_to_ptr_type output_name_to_ptr(creator_key&&){
			return output_name_to_ptr();
		}


		/// \brief Name of the process chain in config file section 'chain'
		std::string const chain;

		/// \brief Name of the module type given via class module_declarant
		std::string const type_name;

		/// \brief Position of the module in the process chain
		///
		/// The first module has number 1.
		std::size_t const number;

		std::string const location;


	protected:
		/// \brief Enables the module for exec calls
		virtual void enable() = 0;

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept = 0;

		/// \brief Make a corresponding module_exec
		virtual module_exec_ptr make_module_exec(
			std::size_t id, output_map_type& output_map) = 0;

		/// \brief Get map from output names to output_base pointers
		virtual output_name_to_ptr_type output_name_to_ptr() = 0;
	};


}


#endif
