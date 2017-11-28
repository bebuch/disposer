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
#include <boost/hana/fold.hpp>


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

		template < typename DimensionList >
		static constexpr void verify_solved(DimensionList const&)noexcept{
			static_assert(
				hana::size(convert< DimensionList >.types) == hana::size_c< 1 >,
				"at least one dimension Ds is not solved yet");
		}

		template < typename DimensionList >
		static constexpr auto calc_type()noexcept{
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			verify_solved(DimensionList{});
#endif

			return convert< DimensionList >.types[hana::size_c< 0 >];
		}

		/// \brief The corresponding type, requires DimensionList to be unqiue
		///        for all Ds
		template < typename DimensionList >
		using type = typename decltype(+calc_type< DimensionList >())::type;
	};

	/// \brief Alias definition without effect
	template < typename T >
	using self_t = T;

	template < template < typename ... > typename >
	struct is_self_t: std::false_type{};

	template <>
	struct is_self_t< self_t >: std::true_type{};

	template < template < typename ... > typename Template >
	constexpr bool is_self_t_v = is_self_t< Template >::value;


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


	// TODO: Use inheritance instead of using to make error messages shorter
	//       and more expressive

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


}


#endif
