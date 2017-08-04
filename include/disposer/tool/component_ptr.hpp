//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__component_ptr__hpp_INCLUDED_
#define _disposer__tool__component_ptr__hpp_INCLUDED_

#include <memory>
#include <functional>
#include <unordered_map>


namespace disposer{


	class component_base;

	/// \brief unique_ptr to class component_base or derived classes
	using component_ptr = std::unique_ptr< component_base >;


	struct component_make_data;

	/// \brief A init function which constructs a component
	using component_maker_fn =
		std::function< component_ptr(component_make_data const&) >;

	/// \brief Map between component type name and its maker function
	using component_maker_list =
		std::unordered_map< std::string, component_maker_fn >;


}


#endif
