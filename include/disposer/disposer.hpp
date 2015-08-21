//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_disposer_hpp_INCLUDED_
#define _disposer_disposer_hpp_INCLUDED_

#include "module_base.hpp"
#include "chain.hpp"

#include <unordered_map>
#include <unordered_set>


namespace disposer{


	using io_list = std::unordered_set< std::string >;

	class disposer{
	public:
		using maker_function = std::function< module_ptr(std::string const& type, std::string const&, std::string const&, io_list const&, io_list const&, parameter_processor&, bool is_start) >;

		void add_module_maker(std::string const& type, maker_function&& function);

		void load(std::string const& filename);

		void trigger(std::string const& chain);

		std::unordered_set< std::string > chains()const;


	private:
		module_ptr make_module(std::string const& type, std::string const& chain, std::string const& name, io_list const& inputs, io_list const& outputs, parameter_processor&& parameters, bool is_start);


		std::unordered_map< std::string, maker_function > maker_list_;

		chain_list chains_;
	};


}


#endif
