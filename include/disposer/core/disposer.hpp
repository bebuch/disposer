//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__disposer__hpp_INCLUDED_
#define _disposer__core__disposer__hpp_INCLUDED_

#include "disposer_declarant.hpp"
#include "disposer_directory.hpp"
#include "chain.hpp"

#include "../config/embedded_config.hpp"

#include <unordered_set>


namespace disposer{


	/// \brief Main class of the disposer software
	class disposer{
	public:
		/// \brief Constructor
		disposer();


		/// \brief Standard Destructor
		~disposer();


		/// \brief Not copyable
		disposer(disposer const&) = delete;

		/// \brief Not movable
		disposer(disposer&&) = delete;


		/// \brief Not copyable
		disposer& operator=(disposer const&) = delete;

		/// \brief Not movable
		disposer& operator=(disposer&&) = delete;


		/// \brief Get a reference to the component_declarant object
		::disposer::component_declarant& component_declarant();

		/// \brief Get a reference to the module_declarant object
		::disposer::module_declarant& module_declarant();


		/// \brief Load and parse the config file
		void load(std::string const& filename);

		/// \brief Create components as in config_ defined
		void create_components();

		/// \brief Create chains as in config_ defined
		void create_chains();


		/// \brief The directory object
		disposer_directory const& directory(){
			return directory_;
		}


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const;


		/// \brief Get a reference to the chain, throw if it does not exist
		chain& get_chain(std::string const& chain);


	private:
		/// \brief Configuration data to create components and chains
		types::embedded_config::config config_;


		/// \brief Component and module generators
		disposer_directory directory_;


		/// \brief List of all components (map from name to object)
		std::unordered_map< std::string, component_ptr > components_;

		/// \brief List of all inactive chains
		std::unordered_set< std::string > inactive_chains_;

		/// \brief List of all active chains (map from name to object)
		std::unordered_map< std::string, chain > chains_;

		/// \brief List of id_generators (map from name to object)
		std::unordered_map< std::string, id_generator > id_generators_;


		/// \brief The declarant object to register new component types
		::disposer::component_declarant component_declarant_;

		/// \brief The declarant object to register new module types
		::disposer::module_declarant module_declarant_;


	friend class module_declarant;
	friend class component_declarant;
	};


}


#endif
