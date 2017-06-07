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

#include "output_base.hpp"


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
		constexpr input_base(
			output_base* output,
			bool last_use
		)noexcept
			:  output_(output)
			, last_use_(last_use)
			, id_(0) {}


		/// \brief Inputs are not copyable
		input_base(input_base const&) = delete;

		/// \brief Inputs are default-movable
		constexpr input_base(input_base&&)noexcept = default;


		/// \brief Inputs are not copy-assingable
		input_base& operator=(input_base const&) = delete;

		/// \brief Inputs are not move-assingble
		input_base& operator=(input_base&&) = delete;


		/// \brief Set the new id for the next exec or cleanup
		void set_id(module_base_key&&, std::size_t id)noexcept{ id_ = id; }

		/// \brief Remove data from connected output if last_use_
		void cleanup(module_base_key&& key, std::size_t id)noexcept{
			if(last_use_ && output_) output_->cleanup(std::move(key), id);
		}


	protected:
		/// \brief Get the current ID
		std::size_t current_id()const noexcept{ return id_; }

		/// \brief Get connected output or nullptr
		output_base* output_ptr()const noexcept{ return output_; }

		/// \brief
		bool last_use()const noexcept{
			return last_use_;
		}


	private:
		/// \brief Pointer to the linked output
		output_base* const output_;

		/// \brief Is this the last use of the connected output?
		bool const last_use_;

		/// \brief The actual ID while module::exec() is running
		std::size_t id_;

	};


}


#endif
