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

#include "input_base.hpp"

#include <utility>


namespace disposer{


	/// \brief Connection between an output and an input
	class signal_t{
	public:
		/// \brief Called by output to move data to receiving inputs
		void operator()(
			std::size_t id, any_type const& data, type_index const& type
		)const{
			for(auto& [input, last_use]: targets_){
				input.add(id, data, type, last_use);
			}
		}

		/// \brief Add an input to the target list
		///
		/// The last_use flag shall be true, if the input is the last which
		/// gets the variable in the chain.
		void connect(input_base& input, bool last_use){
			targets_.emplace_back(input, last_use);
		}

	private:
		/// \brief List of all connected inputs
		std::vector< std::pair< input_base&, bool > > targets_;
	};


	/// \brief Base for module outpus
	///
	/// Polymorphe base class for module outputs.
	///
	/// A disposer module output must have at least one output data type.
	/// An output might have more then one data type.
	class output_base{
	public:
		/// \brief Constructor
		output_base(std::string const& name): name(name), id(id_), id_(0) {}

		/// \brief Outputs are not copyable
		output_base(output_base const&) = delete;

		/// \brief Outputs are not movable
		output_base(output_base&&) = delete;


		/// \brief Outputs are not copyable
		output_base& operator=(output_base const&) = delete;

		/// \brief Outputs are not movable
		output_base& operator=(output_base&&) = delete;


	protected:
		/// \brief List of active output types
		virtual std::vector< type_index > active_types()const = 0;


		/// \brief Name of the output in the config file
		std::string const name;

		signal_t signal;

		/// \brief Read only reference to the actual ID while module::exec()
		///        is running
		std::size_t const& id;


	private:
		/// \brief The actual ID while module::exec() is running
		std::size_t id_;


	friend class module_base;
	friend class disposer::impl;
	};


}


#endif
