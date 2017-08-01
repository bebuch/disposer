//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__disposer__hpp_INCLUDED_
#define _disposer__disposer__hpp_INCLUDED_

#include "component_ptr.hpp"
#include "chain.hpp"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>


namespace disposer{


	struct disposer_key;
	class disposer;


	/// \brief Functor to register a new module by a name and an init function
	class module_declarant{
	public:
		/// \brief Register a new module in the disposer by mapping the
		///        module name to the constructing init function
		void operator()(
			std::string const& type,
			module_maker_fn&& fn);


	private:
		/// \brief Only constructible by the disposer class
		module_declarant(disposer& disposer): disposer_(disposer) {}


		/// \brief Not copyable
		module_declarant(module_declarant const&) = delete;

		/// \brief Not movable
		module_declarant(module_declarant&&) = delete;


		/// \brief Reference to the disposer class
		disposer& disposer_;

	friend class disposer;
	};


	/// \brief Functor to register a new component by a name and an init
	///        function
	class component_declarant{
	public:
		/// \brief Register a new component in the disposer by mapping the
		///        component name to the constructing init function
		void operator()(
			std::string const& type,
			component_maker_fn&& fn);

		/// \brief Get a reference to the disposer object
		::disposer::disposer& disposer(){ return disposer_; }


	private:
		/// \brief Only constructible by the disposer class
		component_declarant(::disposer::disposer& disposer)
			: disposer_(disposer) {}


		/// \brief Not copyable
		component_declarant(component_declarant const&) = delete;

		/// \brief Not movable
		component_declarant(component_declarant&&) = delete;


		/// \brief Reference to the disposer class
		::disposer::disposer& disposer_;

	friend class ::disposer::disposer;
	};


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


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const;


		/// \brief Get a reference to the chain, throw if it does not exist
		chain& get_chain(std::string const& chain);


	private:
		/// \brief List of components (map from component type name to maker
		///        function)
		component_maker_list component_maker_list_;

		/// \brief List of all components (map from name to object)
		std::unordered_map< std::string, component_ptr > components_;

		/// \brief List of modules (map from module type name to maker function)
		module_maker_list module_maker_list_;

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

	friend class ::disposer::module_declarant;
	friend class ::disposer::component_declarant;
	};


}


#endif
