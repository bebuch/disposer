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

#include "chain.hpp"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>


namespace disposer{


	struct make_data;
	class disposer;


	/// \brief Functor to register a new module by a name and an init function
	class module_declarant{
	public:
		/// \brief Register a new module in the disposer by mapping the
		///        module name to the constructing init function
		void operator()(
			std::string const& type,
			module_maker_function&& function);


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


	/// \brief Main class of the disposer software
	class disposer{
	public:
		/// \brief Constructor
		disposer();


		/// \brief Not copyable
		disposer(disposer const&) = delete;

		/// \brief Not movable
		disposer(disposer&&) = delete;


		/// \brief Not copyable
		disposer& operator=(disposer const&) = delete;

		/// \brief Not movable
		disposer& operator=(disposer&&) = delete;


		/// \brief Get a reference to the module_declarant object
		module_declarant& declarant();


		/// \brief Load and parse the config file
		void load(std::string const& filename);


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const;

		/// \brief List of all chaines in group
		std::vector< std::string > chains(std::string const& group)const;


		/// \brief Get a reference to the chain, throw if it does not exist
		chain& get_chain(std::string const& chain);


		/// \brief List of all groups of chains
		///
		/// Groups have no special use in the disposer, they are comfore for
		/// the user, to bind associated chains under one name.
		///
		/// The standard id_generator of group member chains is the group name.
		std::unordered_set< std::string > groups()const;


	private:
		/// \brief List of id_generators (map from name to object)
		std::unordered_map< std::string, id_generator > id_generators_;

		/// \brief List of groups (map from name to chain list)
		std::unordered_map<
			std::string, std::vector< std::reference_wrapper< chain > >
		> groups_;

		/// \brief List of modules (map from module type name to maker function)
		module_maker_list maker_list_;

		/// \brief List of alle chains (map from name to object)
		std::unordered_map< std::string, chain > chains_;

		/// \brief The declarant object to register new module types
		module_declarant declarant_;

	friend class module_declarant;
	};


}


#endif
