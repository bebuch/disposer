//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension_referrer__hpp_INCLUDED_
#define _disposer__core__dimension_referrer__hpp_INCLUDED_

#include "dimension_numbers.hpp"

#include <boost/hana/functional/arg.hpp>
#include <boost/hana/greater.hpp>


namespace disposer{


	/// \brief Defines a type by modules dimensions
	///
	/// \param Template A template that must be instantiable with the same
	///                 number of types as the number of Ds
	/// \param Ds Index of a dimension_list between 0 and N - 1, where N is the
	///           number of Dimension's in a dimension_list
	template < template < typename ... > typename Template, std::size_t ... Ds >
	struct dimension_referrer{
		/// \brief Converts between packed indexes and corresponding types
		template < typename DimensionList >
		static constexpr auto convert
			= dimension_converter< DimensionList, Template, Ds ... >{};
	};

	/// \brief Alias definition without effect
	template < typename T >
	using self_t = T;

	template < typename T >
	struct free_t{
		template < typename ...
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			Args
#endif
		> struct impl{
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			static_assert(sizeof...(Args) == 0, "Args must be empty");
#endif

			using type = T;
		};

		template < typename ... Args >
		using type = typename impl< Args ... >::type;
	};


	/// \brief Wrap the active types of the given dimension's Ds in Template
	template < template < typename ... > typename Template, std::size_t ... Ds >
	constexpr dimension_referrer< Template, Ds ... > wrapped_type_ref_c{};

	/// \brief Defines a type by a module dimension
	template < std::size_t D >
	using type_ref = dimension_referrer< self_t, D >;

	/// \brief Refers to the active type of the given dimension D
	template < std::size_t D >
	constexpr type_ref< D > type_ref_c{};

	/// \brief Defines a type
	template < typename T >
	using free_type = dimension_referrer< free_t< T >::template type >;

	/// \brief Refers to type T
	template < typename T >
	constexpr free_type< T > free_type_c{};


	/// \brief Hana tag for dimension_dependancy
	struct dimension_dependancy_tag{};

	/// \brief Parameter value depends on the named module dimensions
	template < std::size_t ... VDs >
	struct dimension_dependancy{
		using hana_tag = dimension_dependancy_tag;
	};


}


#endif
