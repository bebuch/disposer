//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__input_base__hpp_INCLUDED_
#define _disposer__input_base__hpp_INCLUDED_

#include "type_index.hpp"

#include <string_view>
#include <vector>


namespace disposer{


	struct module_base_key;
	struct creator_key;

	class output_base;


	/// \brief Base for module inputs
	///
	/// Polymorphe base class for module inputs.
	///
	/// A disposer module input must have at least one input data type.
	/// An input might have more then one data type.
	class input_base{
	public:
		/// \brief Constructor
		constexpr input_base(std::string_view name)noexcept:
			name(name), output_(nullptr), id_(0) {}


		/// \brief Inputs are not copyable
		input_base(input_base const&) = delete;

		/// \brief Inputs are not movable
		input_base(input_base&&) = delete;


		/// \brief Inputs are not copy-assingable
		input_base& operator=(input_base const&) = delete;

		/// \brief Inputs are not move-assingble
		input_base& operator=(input_base&&) = delete;


		/// \brief Destructors
		virtual ~input_base() = default;


		/// \brief Get a list of all input data types
		virtual std::vector< type_index > type_list()const = 0;

		/// \brief Enable the given types
		void set_output(creator_key&&, output_base* output)noexcept{
			output_ = output;
		}


		/// \brief Enable the given types
		bool enable_types(
			creator_key&&,
			std::vector< type_index > const& types
		)noexcept{ return enable_types(types); }


		/// \brief Set the new id for the next exec or cleanup
		void set_id(module_base_key&&, std::size_t id)noexcept{ id_ = id; }


		/// \brief Name of the input in the config file
		std::string_view const name;


	protected:
		/// \brief Enable the given types
		virtual bool enable_types(
			std::vector< type_index > const& types
		) noexcept = 0;


		/// \brief Get the current ID
		std::size_t current_id()noexcept{ return id_; }

		/// \brief Get connected output or nullptr
		output_base* output_ptr()noexcept{ return output_; }


	private:
		/// \brief Pointer to the linked output
		output_base* output_;

		/// \brief The actual ID while module::exec() is running
		std::size_t id_;
	};


}


#endif
