//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_exec_base__hpp_INCLUDED_
#define _disposer__core__output_exec_base__hpp_INCLUDED_

#include "../tool/type_index.hpp"
#include "../tool/any_type.hpp"

#include <utility>
#include <vector>
#include <atomic>


namespace disposer{


	struct input_exec_key;


	/// \brief Base for module outpus
	///
	/// Polymorphe base class for module outputs.
	///
	/// A disposer module output must have at least one output data type.
	/// An output might have more then one data type.
	class output_exec_base{
	public:
		/// \brief Constructor
		output_exec_base(std::size_t use_count)noexcept
			: remaining_use_count_(use_count) {}

		/// \brief Outputs are not copyable
		output_exec_base(output_exec_base const&) = delete;

		/// \brief Outputs are not movable
		output_exec_base(output_exec_base&&) = delete;


		/// \brief Outputs are not copy-assignable
		output_exec_base& operator=(output_exec_base const&) = delete;

		/// \brief Outputs are not move-assignable
		output_exec_base& operator=(output_exec_base&&) = delete;


	protected:
		/// \brief true if remaining_use_count_ is 1, false otherwise
		bool is_last_use()const noexcept{ return remaining_use_count_ == 1; }


		/// \brief Call cleanup() if all connected inputs are ready
		bool is_cleanup()noexcept{
			return --remaining_use_count_ == 0;
		}


	private:
		/// \brief Data can only be moved to an input, if all previos
		///        inputs are ready
		std::atomic< std::size_t > remaining_use_count_;
	};


}


#endif
