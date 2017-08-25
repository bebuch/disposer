//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension_converter__hpp_INCLUDED_
#define _disposer__core__dimension_converter__hpp_INCLUDED_

#include "dimension.hpp"

#include <boost/hana/functional/arg.hpp>
#include <boost/hana/greater.hpp>


namespace disposer{


	/// \brief Defines a type by modules dimensions
	///
	/// \param Template A template that must be instantiable with the same
	///                 number of types as the number of D's
	/// \param D Index of a dimension_list between 0 and N - 1, where N is the
	///          number of Dimension's in a dimension_list
	template < template < typename ... > typename Template, std::size_t ... D >
	struct dimension_converter{
		/// \brief Calculate the type by a given dimension_list and the indexes
		///        of it's active types.
		template < typename ... Dimension, std::size_t ... I >
		static auto convert(
			dimension_list< Dimension ... >,
			hana::tuple< hana::size_t< I > ... >
		){
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			static_assert(sizeof...(Dimension) == sizeof...(I));
#endif

			static_assert(hana::all_of(hana::tuple_c< std::size_t, D ... >,
				hana::curry< 2 >(hana::greater)(
					hana::size_c< sizeof...(Dimension) >)),
				"At least one wrapped_type_ref index D is greater or equal "
				"as the module dimension count. (first index is 0)");

			return hana::type_c< Template<
				typename decltype(+hana::arg< D + 1 >(
					Dimension::types[hana::size_c< I >] ...))::type ... > >;
		}
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


	/// \brief Wrap the active types of the given dimension's D in Template
	template < template < typename ... > typename Template, std::size_t ... D >
	constexpr dimension_converter< Template, D ... > wrapped_type_ref_c{};

	/// \brief Defines a type by a module dimension
	template < std::size_t D >
	using type_ref = dimension_converter< self_t, D >;

	/// \brief Refers to the active type of the given dimension D
	template < std::size_t D >
	constexpr type_ref< D > type_ref_c{};

	/// \brief Defines a type
	template < typename T >
	using free_type = dimension_converter< free_t< T >::template type >;

	/// \brief Refers to type T
	template < typename T >
	constexpr free_type< T > free_type_c{};


}


#endif
