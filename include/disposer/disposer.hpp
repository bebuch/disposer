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
		using maker_function = std::function< module_ptr(make_data&) >;

		void operator()(std::string const& type, maker_function&& function);


	private:
		module_adder(disposer& disposer): disposer_(disposer) {}

		module_adder(module_adder const&) = delete;
		module_adder(module_adder&&) = delete;

		disposer& disposer_;

	friend class disposer;
	};

	class disposer{
	public:
		using maker_function = module_adder::maker_function;

		disposer();

		disposer(disposer const&) = delete;
		disposer(disposer&&) = delete;

		disposer& operator=(disposer const&) = delete;
		disposer& operator=(disposer&&) = delete;


		module_adder& adder();

		void load(std::string const& filename);

		void trigger(std::string const& chain);

		std::unordered_set< std::string > chains()const;

		std::vector< std::string > chains(std::string const& group)const;

		std::unordered_set< std::string > groups()const;


		class impl;


	private:
		std::unordered_map< std::string, id_generator > id_generators_;

		std::unordered_map<
			std::string, std::vector< std::reference_wrapper< chain > >
		> groups_;

		std::unordered_map< std::string, maker_function > maker_list_;

		std::unordered_map< std::string, chain > chains_;

		module_adder adder_;

	friend class module_adder;
	};


}


#endif
