//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input_base__hpp_INCLUDED_
#define _disposer__core__input_base__hpp_INCLUDED_


namespace disposer{


	class output_base;


	/// \brief Base for module inputs
	///
	/// Polymorphe base class for module inputs.
	///
	/// A module input must have at least one input data type.
	/// An input might have more then one data type.
	class input_base{
	public:
		/// \brief Constructor
		input_base(output_base* output)noexcept
			: output_(output) {}


		/// \brief Inputs are not copyable
		input_base(input_base const&) = delete;

		/// \brief Inputs are movable
		input_base(input_base&&) = default;


		/// \brief Inputs are not copy-assingable
		input_base& operator=(input_base const&) = delete;

		/// \brief Inputs are not move-assingble
		input_base& operator=(input_base&&) = delete;


		/// \brief Get connected output or nullptr
		output_base* output_ptr()const noexcept{ return output_; }


	private:
		/// \brief Pointer to the linked output
		output_base* const output_;
	};


}


#endif
