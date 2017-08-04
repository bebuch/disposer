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
		constexpr input_exec_base(
			output_exec_base* output,
			std::size_t use_count
		)noexcept
			: output_(output)
			, remaining_use_count_(use_count) {}


		/// \brief Inputs are not copyable
		input_exec_base(input_exec_base const&) = delete;

		/// \brief Inputs are not movable
		input_exec_base(input_exec_base&&) = delete;


		/// \brief Inputs are not copy-assingable
		input_exec_base& operator=(input_exec_base const&) = delete;

		/// \brief Inputs are not move-assingble
		input_exec_base& operator=(input_exec_base&&) = delete;


		/// \brief Remove data from connected output if remaining_use_count_
		///        drops to 0
		void cleanup(module_base_key&&)noexcept{
			if(output_ && --remaining_use_count_ == 0){
				output_->cleanup(input_exec_key());
			}
		}


	protected:
		/// \brief Get connected output or nullptr
		output_exec_base* output_ptr()const noexcept{ return output_; }

		/// \brief true if remaining_use_count_ is 1, false otherwise
		bool is_last_use()noexcept const{ return remaining_use_count_ == 1; }


	private:
		/// \brief Pointer to the linked output
		output_exec_base* const output_;

		/// \brief Data can only be moved to an output, if all previos
		///        outputs are ready
		std::atomic< std::size_t > remaining_use_count_;
	};


}


#endif
