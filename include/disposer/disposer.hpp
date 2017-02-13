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

	class module_adder{
	public:
		/// \brief A init function which constructs a module
		using maker_function = std::function< module_ptr(make_data&) >;

		/// \brief Register a new module in the disposer by mapping the
		///        module name to the constructing init function
		void operator()(std::string const& type, maker_function&& function);


	private:
		/// \brief Only constructible by the disposer class
		module_adder(disposer& disposer): disposer_(disposer) {}


		/// \brief Not copyable
		module_adder(module_adder const&) = delete;

		/// \brief Not movable
		module_adder(module_adder&&) = delete;


		/// \brief Reference to the disposer class
		disposer& disposer_;

	friend class disposer;
	};


	class disposer{
	public:
		/// \copydoc module_adder::maker_function
		using maker_function = module_adder::maker_function;


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


		/// \brief Get a reference to the module_adder object
		module_adder& adder();

		/// \brief Load and parse the config file
		void load(std::string const& filename);

		/// \brief Execute the given chain as defined in the config file
		void exec(std::string const& chain);

		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const;

		/// \brief List of all chaines in group
		std::vector< std::string > chains(std::string const& group)const;

		/// \brief List of all groups of chains
		///
		/// Groups have no special use in the disposer, they are comfore for
		/// the user, to bind associated chains under one name.
		///
		/// The standard id_generator of group member chains is the group name.
		std::unordered_set< std::string > groups()const;


		class impl;


	private:
		/// \brief List of id_generators (map from name to object)
		std::unordered_map< std::string, id_generator > id_generators_;

		/// \brief List of groups (map from name to chain list)
		std::unordered_map<
			std::string, std::vector< std::reference_wrapper< chain > >
		> groups_;

		/// \brief List of modules (map from module type name to maker function)
		std::unordered_map< std::string, maker_function > maker_list_;

		/// \brief List of alle chains (map from name to object)
		std::unordered_map< std::string, chain > chains_;

		/// \brief The adder object to register new module types
		module_adder adder_;

	friend class module_adder;
	};


}


#endif
