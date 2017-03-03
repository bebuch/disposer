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

#include <boost/functional/hash.hpp>
#include <boost/type_index/ctti_type_index.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>


namespace std{

	// make ctti_type_index hashable for unordered_map and unordered_set
	template <> struct hash< boost::typeindex::ctti_type_index >:
		boost::hash< boost::typeindex::ctti_type_index >{};

}


namespace disposer{


	using type_index = boost::typeindex::ctti_type_index;


	/// \brief Dummy type for references to real data in inputs and outputs
	struct any_type;


	/// \brief Class module_base access key
	struct module_base_key{
	private:
		/// \brief Constructor
		constexpr module_base_key()noexcept = default;
		friend class module_base;
	};

	/// \brief Class signal_t access key
	struct signal_t_key{
	private:
		/// \brief Constructor
		constexpr signal_t_key()noexcept = default;
		friend class signal_t;
	};


	struct creator_key;

	/// \brief Defined in create_chain_modules.cpp
	creator_key make_creator_key();

	/// \brief Access key creator functions
	struct creator_key{
	private:
		/// \brief Constructor
		constexpr creator_key()noexcept = default;
		friend creator_key make_creator_key();
	};



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


		/// \brief Get a list of all input data types
		virtual std::vector< type_index > type_list()const = 0;

		/// \brief Enable the given types
		bool enable_types(
			creator_key,
			std::vector< type_index > const& types
		) noexcept{ return enable_types(types); }


		/// \brief Call add(id, value, type, last_use)
		void add(
			signal_t_key,
			std::size_t id,
			any_type const& value,
			type_index const& type,
			bool last_use
		){ add(id, value, type, last_use); }


		/// \brief Set the new id for the next exec or cleanup
		void set_id(module_base_key, std::size_t id)noexcept{ id_ = id; }

		/// \brief Call cleanup(id)
		void cleanup(module_base_key, std::size_t id)noexcept{ cleanup(id); }



		/// \brief Name of the input in the config file
		std::string const name;


	protected:
		/// \brief Add data to an input
		virtual void add(
			std::size_t id,
			any_type const& value,
			type_index const& type,
			bool last_use
		) = 0;


		/// \brief Enable the given types
		virtual bool enable_types(
			std::vector< type_index > const& types
		) noexcept = 0;



		/// \brief Clean up all data with ID less or equal id
		virtual void cleanup(std::size_t id)noexcept = 0;

		/// \brief Read only reference to the actual ID while module::exec()
		///        is running
		std::size_t const& id;


	private:
		/// \brief The actual ID while module::exec() is running
		std::size_t id_;
	};


}


#endif
