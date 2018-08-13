//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__optional_component__hpp_INCLUDED_
#define _disposer__core__optional_component__hpp_INCLUDED_

#include <cstddef>


namespace disposer{


	/// \brief Reference to a component or empty struct
	template < typename ComponentRef >
	struct optional_component{
		constexpr optional_component(
			ComponentRef& ref,
			std::size_t& usage_count
		)noexcept
			: component(ref)
			, usage_count(usage_count) {}

		constexpr optional_component(optional_component const& other)noexcept
			: component(other.component)
			, usage_count(other.usage_count) {}


		using ref_type = ComponentRef;

		ref_type component;

		std::size_t& usage_count;
	};

	/// \brief Empty struct
	template <>
	struct optional_component< void >{
		using ref_type = void;
	};


}


#endif
