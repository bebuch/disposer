//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension__hpp_INCLUDED_
#define _disposer__core__dimension__hpp_INCLUDED_

#include <boost/hana/functional/arg.hpp>
#include <boost/hana/core/is_a.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/traits.hpp>
#include <boost/hana/any_of.hpp>
#include <boost/hana/all_of.hpp>
#include <boost/hana/length.hpp>
#include <boost/hana/greater.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/set.hpp>


namespace disposer{


	namespace hana = boost::hana;

	/// \brief hana tag for \ref dimension
	struct dimension_tag{};

	/// \brief Type list of a module
	template < typename ... T >
	struct dimension{
		static_assert(sizeof...(T) > 1,
			"dimension's must contain at least 2 different types");

		/// \brief Types as hana::tuple_t
		static constexpr auto types = hana::tuple_t< T ... >;

		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"dimension types must not contain duplicates");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"dimension types must not be references");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"dimension types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_array),
			"dimension types must not be c-arrays");

		static_assert(!hana::any_of(types, hana::traits::is_void),
			"dimension types must not be void");


		/// \brief Tag for boost::hana
		using hana_tag = dimension_tag;


		/// \brief Assign is forbidden
		dimension& operator=(dimension const&) = delete;
	};


	/// \brief Creates a dimension object
	template < typename ... T >
	constexpr dimension< T ... > dimension_c{};


	/// \brief List of type lists of a module
	template < typename ... Dimension >
	struct dimension_list{
		static_assert(hana::all_of(hana::make_tuple(Dimension() ...),
			hana::is_a< dimension_tag >),
			"Dimension must be a disposer::dimension< ... >");

		/// \brief Tuple of type lists, Dimension is always a \ref dimension
		static constexpr auto dimensions =
			hana::make_tuple(Dimension::types ...);

		/// \brief Default constructor
		///
		/// Disabled if Dimension is empty to resolve ambiguity.
		template < typename Dummy = void, typename = std::enable_if_t<
			(sizeof...(Dimension) > 0), Dummy > >
		constexpr dimension_list()noexcept{}

		/// \brief Constructor
		constexpr dimension_list(Dimension const& ...)noexcept{}
	};


	/// \brief Defines a type by modules dimensions
	///
	/// \param Template A template that must be instantiable with the same
	///                 number of types as the number of D's
	/// \param D Index of a dimension_list between 0 and N - 1, where N is the
	///          number of Dimension's in a dimension_list
	template < template < typename ... > typename Template, std::size_t ... D >
	struct wrapped_type_ref{
		static_assert(sizeof...(D) > 0,
			"At least one type list must be referenced. (need at least one D)");

		/// \brief Calculate the type by a given dimension_list and the indices
		///        of it's active types.
		template < typename ... Dimension, std::size_t ... I >
		static auto target_type(
			dimension_list< Dimension ... >,
			hana::tuple< hana::size_t< I > ... >
		){
			// disposer assert, disable in production code
			static_assert(sizeof...(Dimension) == sizeof...(I));

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

	/// \brief Wrap the active types of the given dimension's D in Template
	template < template < typename ... > typename Template, std::size_t ... D >
	constexpr wrapped_type_ref< Template, D ... > wrapped_type_ref_c{};

	/// \brief Defines a type by a module dimension
	template < std::size_t D >
	using type_ref = wrapped_type_ref< self_t, D >;

	/// \brief Refers to the active type of the given dimension D
	template < std::size_t D >
	constexpr type_ref< D > type_ref_c{};


}


#endif
