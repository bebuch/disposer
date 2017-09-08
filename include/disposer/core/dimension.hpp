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

#include "../tool/type_index.hpp"

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/traits.hpp>
#include <boost/hana/any_of.hpp>
#include <boost/hana/all_of.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/set.hpp>


namespace disposer{


	namespace hana = boost::hana;

	/// \brief hana tag for \ref dimension
	struct dimension_tag{};

	/// \brief Type list of a module
	template < typename ... Ts >
	struct dimension{
		/// \brief Count of types
		static constexpr std::size_t type_count = sizeof...(Ts);

		static_assert(type_count > 1,
			"dimension's must contain at least 2 different types");

		/// \brief Types as hana::tuple_t
		static constexpr auto types = hana::tuple_t< Ts ... >;

		static_assert(hana::size(types) == hana::size(hana::to_set(types)),
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


		/// \brief Array of the type indexes of the Ts
		static constexpr type_index ti[sizeof...(Ts)] =
			{ type_index::type_id< Ts >() ... };
	};


	/// \brief Creates a dimension object
	template < typename ... Ts >
	constexpr dimension< Ts ... > dimension_c{};


	/// \brief List of type lists of a module
	template < typename ... Dimension >
	struct dimension_list{
		static_assert(hana::all_of(
			hana::make_tuple(Dimension{} ...), hana::is_a< dimension_tag >),
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


	/// \brief Pair of a dimension number and the index of its active type
	template < std::size_t D, std::size_t I >
	struct ct_index_component{
		/// \brief Dimension number
		static constexpr auto d = hana::size_c< D >;

		/// \brief Index of the active type of the dimension
		static constexpr auto i = hana::size_c< I >;
	};

	/// \brief List of type lists of a module
	template < typename ... Dimension >
	struct partial_deduced_dimension_list{
		static_assert(hana::all_of(
			hana::make_tuple(Dimension{} ...), [](auto const& dim){
				return hana::is_a< dimension_tag >(dim)
					|| hana::is_a< hana::type_tag >(dim);
			}),
			"Dimension must be a disposer::dimension< ... > or a hana::type");

		/// \brief Tuple of type lists, Dimension is always a \ref dimension
		static constexpr auto dimensions = hana::make_tuple([](auto dim){
				if constexpr(hana::is_a< dimension_tag >(dim)){
					return dim.types;
				}else{
					return dim;
				}
			}(Dimension{}) ...);

		/// \brief Constructor
		constexpr partial_deduced_dimension_list(
			dimension_list< Dimension ... >
		)noexcept{}

		/// \brief Constructor
		constexpr partial_deduced_dimension_list()noexcept = default;
	};

	template < typename ... DLs, std::size_t D, std::size_t I >
	auto make_partial_deduced_dimension_list(
		partial_deduced_dimension_list< DLs ... >,
		ct_index_component< D, I >
	){
		return hana::unpack(hana::range_c< std::size_t, 0, sizeof...(DLs) >,
			[](auto ... n){
				auto const calc = [](auto n, auto dim){
						if constexpr(n.value == Ds){
							return hana::basic_type(dim.types[ic->i]);
						}else{
							return dim;
						}
					};

				return partial_deduced_dimension_list<
					decltype(calc(n, DLs{})) ... >{};
			});
	}


}


#endif
