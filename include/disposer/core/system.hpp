//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__system__hpp_INCLUDED_
#define _disposer__core__system__hpp_INCLUDED_

#include "directory.hpp"
#include "chain.hpp"

#include "../config/parse_config.hpp"

#include <unordered_set>


namespace disposer{


	/// \brief Main class of the disposer software
	class system{
	public:
		/// \brief Constructor
		system() = default;

		/// \brief Destructor
		~system();


		/// \brief Not copyable
		system(system const&) = delete;

		/// \brief Not movable
		system(system&&) = delete;


		/// \brief Not copyable
		system& operator=(system const&) = delete;

		/// \brief Not movable
		system& operator=(system&&) = delete;


		/// \brief Create components and modules by a config file
		///
		/// This function calls load_config(content).
		void load_config_file(std::string const& filename);


		/// \brief Create components and modules
		///
		/// This function must be executed only one time before any other load
		/// and remove function.
		void load_config(std::istream& content);

		/// \brief Remove a component
		void remove_component(std::string const& name);

		/// \brief Create a component
		void load_component(std::istream& content);

		/// \brief Remove a chain
		void remove_chain(std::string const& name);

		/// \brief Create a chain
		void load_chain(std::istream& content);


		/// \brief The directory object
		disposer::directory& directory(){
			return directory_;
		}

		/// \brief The directory object
		disposer::directory const& directory()const{
			return directory_;
		}


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const;


		/// \brief Get a reference to the chain, throw if it does not exist
		chain& get_chain(std::string const& chain);


	private:
		/// \brief Mutex
		mutable std::mutex mutex_;

		/// \brief true after construction, false after the first call of
		///        a load or remove function
		bool load_config_file_valid_ = true;


		/// \brief Component and module generators
		disposer::directory directory_;


		/// \brief Currect configuration
		types::parse::config config_;


		/// \brief List of all components (map from name to object)
		std::unordered_map< std::string, component_ptr > components_;

		/// \brief List of all inactive chains
		std::unordered_set< std::string > inactive_chains_;

		/// \brief List of all active chains (map from name to object)
		std::unordered_map< std::string, chain > chains_;

		/// \brief List of id_generators (map from name to object)
		std::unordered_map< std::string, id_generator > id_generators_;
	};


}


#endif
