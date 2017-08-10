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


	/// \brief Transfer reference to data from output to input
	struct reference_carrier{
		reference_carrier(
			type_index const& type,
			any_type const& data
		): type(type), data(data) {}

		/// \brief Type of the data at runtime
		type_index type;

		/// \brief Typeless reference to the data
		any_type const& data;
	};

	/// \brief Transfer data (inclusive ownership) from output to input
	struct value_carrier{
		value_carrier(
			type_index const& type,
			any_type&& data
		): type(type), data(std::move(data)) {}

		/// \brief Type of the data at runtime
		type_index type;

		/// \brief Typeless reference to the data
		any_type&& data;
	};


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


		/// \brief Call cleanup() if all connected inputs are ready
		void cleanup(input_exec_key&&)noexcept{
			if(--remaining_use_count_ == 0) cleanup();
		}


		/// \brief Call get_references()
		std::vector< reference_carrier > get_references(input_exec_key&&)const{
			return get_references();
		}

		/// \brief Call get_values()
		std::vector< value_carrier > get_values(input_exec_key&&){
			return get_values();
		}


		/// \brief Returns the output name
		virtual std::string_view get_name()const noexcept = 0;


	protected:
		/// \brief Get vector of references with all data
		virtual std::vector< reference_carrier > get_references()const = 0;

		/// \brief Get vector of values with all data
		virtual std::vector< value_carrier > get_values() = 0;

		/// \brief Clean up all data
		virtual void cleanup()noexcept = 0;


		/// \brief true if remaining_use_count_ is 1, false otherwise
		bool is_last_use()const noexcept{ return remaining_use_count_ == 1; }


	private:
		/// \brief Data can only be moved to an input, if all previos
		///        inputs are ready
		std::atomic< std::size_t > remaining_use_count_;
	};


}


#endif
