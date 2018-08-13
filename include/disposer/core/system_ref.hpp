//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__system_ref__hpp_INCLUDED_
#define _disposer__core__system_ref__hpp_INCLUDED_

#include "system.hpp"


namespace disposer{


	/// \brief Same interface as system, but without load_config and with a
	///        check that the component does not remove itself
	class system_ref{
	public:
		/// \brief Constructor
		system_ref(
			system& system,
			std::string_view component_name
		)
			: system_(system)
			, component_name_(component_name) {}

		/// \brief Copy constructor
		system_ref(system_ref const& other)
			: system_(other.system_)
			, component_name_(other.component_name_) {}


		/// \brief Remove a component
		void remove_component(std::string const& name){
			if(name == component_name_){
				throw std::logic_error("component(" + name
					+ ") tried to remove itself");
			}
			system_.remove_component(name);
		}

		/// \brief Create a component
		void load_component(std::istream& content){
			system_.load_component(content);
		}

		/// \brief Remove a chain
		void remove_chain(std::string const& name){
			system_.remove_chain(name);
		}

		/// \brief Create a chain
		void load_chain(std::istream& content){
			system_.load_chain(content);
		}


		/// \brief The directory object
		disposer::directory& directory()const{
			return system_.directory();
		}


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const{
			return system_.chains();
		}


		/// \brief Get a reference to the chain, throw if it does not exist
		chain& get_chain(std::string const& chain){
			return system_.get_chain(chain);
		}


		/// \brief Name of the component
		std::string_view component_name()const noexcept{
			return component_name_;
		}


	private:
		/// \brief Reference to the actual system
		system& system_;

		/// \brief Name of the component
		std::string_view component_name_;
	};


}


#endif
