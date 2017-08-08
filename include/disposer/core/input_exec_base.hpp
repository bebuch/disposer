//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input_exec_base__hpp_INCLUDED_
#define _disposer__core__input_exec_base__hpp_INCLUDED_

#include "output_exec_base.hpp"


namespace disposer{


	class input_exec_base;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class input_exec;

	/// \brief Class input_exec_key access key
	struct input_exec_key{
	private:
		/// \brief Constructor
		constexpr input_exec_key()noexcept = default;

		friend class input_exec_base;

		template < typename Name, typename TypeTransformFn, typename ... T >
		friend class input_exec;
	};


	struct module_base_key;


	/// \brief Base for module inputs
	///
	/// Polymorphe base class for module inputs.
	///
	/// A disposer module input must have at least one input data type.
	/// An input might have more then one data type.
	class input_exec_base{
	public:
		/// \brief Constructor
		input_exec_base(output_exec_base* output)noexcept
			: output_(output) {}


		/// \brief Inputs are not copyable
		input_exec_base(input_exec_base const&) = delete;

		/// \brief Inputs are not movable
		input_exec_base(input_exec_base&&) = delete;


		/// \brief Inputs are not copy-assingable
		input_exec_base& operator=(input_exec_base const&) = delete;

		/// \brief Inputs are not move-assingble
		input_exec_base& operator=(input_exec_base&&) = delete;


		/// \brief Tell the connected output that this input finished
		void cleanup(module_base_key&&)noexcept{
			if(output_) output_->cleanup(input_exec_key());
		}


	protected:
		/// \brief Get connected output or nullptr
		output_exec_base* output_ptr()const noexcept{ return output_; }


	private:
		/// \brief Pointer to the linked output
		output_exec_base* const output_;
	};


}


#endif
