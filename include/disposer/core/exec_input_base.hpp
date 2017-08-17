//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_input_base__hpp_INCLUDED_
#define _disposer__core__exec_input_base__hpp_INCLUDED_

#include "exec_output_base.hpp"


namespace disposer{


	class exec_input_base;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class exec_input;

	/// \brief Class exec_input_key access key
	struct exec_input_key{
	private:
		/// \brief Constructor
		constexpr exec_input_key()noexcept = default;

		friend class exec_input_base;

		template < typename Name, typename TypeTransformFn, typename ... T >
		friend class exec_input;
	};


	/// \brief Base for module inputs
	///
	/// Polymorphe base class for module inputs.
	///
	/// A disposer module input must have at least one input data type.
	/// An input might have more then one data type.
	class exec_input_base{
	public:
		/// \brief Constructor
		exec_input_base(exec_output_base* output)noexcept
			: output_(output) {}

		/// \brief Inputs are not copyable
		exec_input_base(exec_input_base const&) = delete;

		/// \brief Inputs are not movable
		exec_input_base(exec_input_base&&) = delete;


		/// \brief Inputs are not copy-assingable
		exec_input_base& operator=(exec_input_base const&) = delete;

		/// \brief Inputs are not move-assingble
		exec_input_base& operator=(exec_input_base&&) = delete;


	protected:
		/// \brief Get connected output or nullptr
		exec_output_base* output_ptr()const noexcept{ return output_; }


	private:
		/// \brief Pointer to the linked output
		exec_output_base* const output_;
	};


}


#endif
