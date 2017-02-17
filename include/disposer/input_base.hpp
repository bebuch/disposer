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

#include "disposer.hpp"

#include <boost/type_index.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>


namespace disposer{


	using boost::typeindex::type_index;


	/// \brief Dummy type for references to real data in inputs and outputs
	struct any_type;


	/// \brief Base for module inputs
	///
	/// Polymorphe base class for module inputs.
	///
	/// A disposer module input must have at least one input data type.
	/// An input might have more then one data type.
	class input_base{
	public:
		/// \brief Constructor
		input_base(std::string const& name): name(name), id(id_), id_(0) {}


		/// \brief Inputs are not copyable
		input_base(input_base const&) = delete;

		/// \brief Inputs are not movable
		input_base(input_base&&) = delete;


		/// \brief Inputs are not copyable
		input_base& operator=(input_base const&) = delete;

		/// \brief Inputs are not movable
		input_base& operator=(input_base&&) = delete;


		/// \brief Destructors
		virtual ~input_base() = default;


	protected:
		/// \brief Add data to an input
		virtual void add(
			std::size_t id,
			any_type const& value,
			type_index const& type,
			bool last_use
		) = 0;

		/// \brief Clean up all data with ID less or equal id
		virtual void cleanup(std::size_t id)noexcept = 0;

		/// \brief Get a list of all active input data types
		virtual bool enable_types(
			std::vector< type_index > const& types
		) noexcept = 0;

		/// \brief Get a list of all input data types
		virtual std::vector< type_index > types()const = 0;


		/// \brief Name of the input in the config file
		std::string const name;

		/// \brief Read only reference to the actual ID while module::exec()
		///        is running
		std::size_t const& id;


	private:
		/// \brief The actual ID while module::exec() is running
		std::size_t id_;


	friend class signal_t;
	friend class module_base;
	friend class disposer::impl;
	};


}


#endif
