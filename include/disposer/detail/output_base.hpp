//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__output_base__hpp_INCLUDED_
#define _disposer__output_base__hpp_INCLUDED_

#include "any_type.hpp"
#include "type_index.hpp"

#include <boost/hana.hpp>

#include <utility>
#include <vector>
#include <stdexcept>
#include <string_view>


namespace disposer{


	namespace hana = boost::hana;


	struct module_base_key;
	struct input_key;


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
	class output_base{
	public:
		using TransferFn =
			std::function< void(std::vector< value_carrier >&&) >;

		/// \brief Constructor
		constexpr output_base()noexcept: id_(0) {}

		/// \brief Outputs are not copyable
		output_base(output_base const&) = default;

		/// \brief Outputs are default-movable
		constexpr output_base(output_base&& other)noexcept = default;


		/// \brief Outputs are not copy-assignable
		output_base& operator=(output_base const&) = delete;

		/// \brief Outputs are not move-assignable
		output_base& operator=(output_base&&) = delete;


		/// \brief List of enabled output types
		virtual std::map< type_index, bool > enabled_types()const = 0;


		/// \brief Set id and call prepare()
		void prepare(module_base_key&&, std::size_t id)noexcept{
			id_ = id;
			prepare();
		}

		/// \brief Call cleanup(id)
		void cleanup(module_base_key&&, std::size_t id)noexcept{
			cleanup(id);
		}


		/// \brief Call get_references(id)
		std::vector< reference_carrier >
		get_references(input_key&&, std::size_t id)const{
			return get_references(id);
		}

		/// \brief Call get_values(id)
		std::vector< value_carrier >
		get_values(input_key&&, std::size_t id){
			return get_values(id);
		}


		/// \brief Returns the output name
		virtual std::string_view get_name()const noexcept = 0;


	protected:
		/// \brief Get vector of references to all data with id
		///
		/// This is called, if data is refered via reference or copied from
		/// output to input.
		virtual std::vector< reference_carrier >
		get_references(std::size_t id)const = 0;

		/// \brief Get vector of values with all data with id
		///
		/// This is called, if data is moved from output to input.
		virtual std::vector< value_carrier >
		get_values(std::size_t id) = 0;

		/// \brief Prepare output for exec
		virtual void prepare()noexcept = 0;

		/// \brief Clean up all data with ID
		virtual void cleanup(std::size_t id)noexcept = 0;


		/// \brief Get the current ID
		std::size_t current_id()noexcept{ return id_; }


	private:
		/// \brief The current ID while module::exec() is running
		std::size_t id_;
	};


}


#endif
