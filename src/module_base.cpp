//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/module_base.hpp>


namespace disposer{


	module_base::module_base(make_data const& data):
		type_name(data.type_name),
		chain(data.chain),
		name(data.name),
		number(data.number),
		id(id_),
		id_(0)
		{}

	void module_base::cleanup(std::size_t id)noexcept{
		for(auto& input: inputs){
			input.second.cleanup(id);
		}
	}


}
