//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__disposer__hpp_INCLUDED_
#define _disposer__disposer__hpp_INCLUDED_

#include "module_base.hpp"
#include "chain.hpp"

#include <unordered_map>


namespace disposer{


	class disposer{
	public:
		using maker_function = std::function< module_ptr(make_data&) >;

		disposer() = default;

		disposer(disposer const&) = delete;
		disposer(disposer&&) = delete;

		disposer& operator=(disposer const&) = delete;
		disposer& operator=(disposer&&) = delete;
	

		void add_module_maker(std::string const& type, maker_function&& function);

		void load(std::string const& filename);

		void trigger(std::string const& chain);

		std::unordered_set< std::string > chains()const;

		std::unordered_set< std::string > chains(std::string const& group)const;

		std::unordered_set< std::string > groups()const;


	private:
		module_ptr make_module(make_data&& data);


		std::unordered_map< std::string, id_generator > id_generators_;

		std::unordered_map< std::string, std::vector< std::reference_wrapper< chain > > > groups_;

		std::unordered_map< std::string, maker_function > maker_list_;

		std::unordered_map< std::string, chain > chains_;
	};


}


#endif
