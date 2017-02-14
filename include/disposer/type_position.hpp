//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__type_position__hpp_INCLUDED_
#define _disposer__type_position__hpp_INCLUDED_

#include "container_lists.hpp"


namespace disposer::detail{


	/// \brief Implementation class of type_position_v
	template < typename Ref, std::size_t Index, typename ... Tests >
	struct type_position;

	/// \brief Implementation class of type_position_v
	template < std::size_t Index, bool valid >
	struct type_position_test: std::integral_constant< std::size_t, Index >{};

	/// \brief Implementation class of type_position_v
	template < std::size_t Index >
	struct type_position_test< Index, false >{
		static_assert(Index == -1, "type_position did not find the type");
	};

	/// \brief Implementation class of type_position_v
	template < typename Ref, std::size_t Index, typename Test >
	struct type_position< Ref, Index, Test >:
		type_position_test< Index, std::is_same< Ref, Test >::value >{};

	/// \brief Implementation class of type_position_v
	template <
		typename Ref,
		std::size_t Index,
		typename Test,
		typename ... Tests
	>
	struct type_position< Ref, Index, Test, Tests ... >: std::conditional_t<
		std::is_same< Ref, Test >::value,
		std::integral_constant< std::size_t, Index >,
		type_position< Ref, 1 + Index, Tests ... >
	>{};

	/// \brief Implementation class of type_position_v
	template < typename Ref, std::size_t Index, typename ... Tests >
	struct type_position< Ref, Index, type_list< Tests ... > >:
		type_position< Ref, Index, Tests ... >{};


}


namespace disposer{


	/// \brief Get index of Ref in Tests
	template < typename Ref, typename ... Tests >
	constexpr std::size_t type_position_v =
		detail::type_position< Ref, 0, Tests ... >::value;


}


#endif
