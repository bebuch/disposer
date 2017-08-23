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
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/keys.hpp>
#include <boost/hana/remove_if.hpp>
#include <boost/hana/is_empty.hpp>
#include <boost/hana/unique.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/sort.hpp>

#include <string>
#include <sstream>


namespace disposer{


	namespace hana = boost::hana;

	/// \brief hana tag for \ref dimension
	struct dimension_tag{};

	/// \brief Type list of a module
	template < typename ... T >
	struct dimension{
		/// \brief Count of types
		static constexpr std::size_t type_count = sizeof...(T);

		static_assert(type_count > 1,
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


	/// \brief Pair of a dimension number and the index of its active type
	template < std::size_t D >
	struct dimension_index{
		/// \brief Dimension number
		static constexpr std::size_t d = D;

		/// \brief Index of the active type of the dimension
		std::size_t i;
	};

	/// \brief Compile time pair of a dimension number and the index of its
	///        active type
	template < std::size_t D, std::size_t I >
	struct ct_dimension_index{
		/// \brief Dimension number
		static constexpr std::size_t d = D;

		/// \brief Index of the active type of the dimension
		static constexpr std::size_t i = I;
	};

	template < bool EvaluationSucceed, typename ... CtDimensionIndex >
	struct solved_dimensions;

	template <>
	struct solved_dimensions< false >: std::false_type{};

	template < typename ... CtDimensionIndex >
	struct solved_dimensions< true, CtDimensionIndex ... >: std::true_type{
		static constexpr std::size_t index_count = sizeof...(CtDimensionIndex);

		static constexpr auto indexes =
			hana::make_tuple(CtDimensionIndex{} ...);
	};


	namespace detail{


		template < std::size_t D >
		static constexpr auto equal_to(hana::size_t< D >){
			return [](auto i){ return i == hana::size_c< D >; };
		}

		template < std::size_t D, std::size_t ... V >
		static constexpr auto index_of(
			hana::tuple< hana::size_t< V > ... >,
			hana::size_t< D >
		){
			return hana::index_if(
				hana::tuple_c< std::size_t, V ... >,
				equal_to(hana::size_c< D >)).value();
		}


	}


	/// \brief Tool to evaluate active dimension of inputs if possible
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... D >
	struct dimension_solver{
		/// \brief Dimension list as hana::tuple of dimensions
		static constexpr auto dimensions = DimensionList::dimensions;

		/// \brief Sorted and summarized dimension numbers
		static constexpr auto ds = hana::unique(hana::sort(
				hana::tuple_c< std::size_t, D ... >));

		template <
			typename ValueType,
			std::size_t ... KD, std::size_t ... KI >
		static constexpr auto solve(
			hana::basic_type< ValueType > known_value_type,
			hana::tuple< ct_dimension_index< KD, KI > ... > known_indexes
		){
			constexpr auto keys = hana::unpack(ds, [](auto ... d){
					return hana::cartesian_product(
						hana::make_tuple(hana::make_range(
							hana::size_c< 0 >,
							hana::size(dimensions[d])
						) ...));
				});

			// get all keys that nighter disagree with the known value_type nor
			// with a known index
			constexpr auto remaining_keys = hana::remove_if(keys,
				[known_value_type, known_indexes](auto const key){
					auto const value_type =
						hana::type_c< Template< typename decltype(
								+dimensions[hana::size_c< D >][key[
									detail::index_of(ds, hana::size_c< D >)]]
							)::type ... > >;
					constexpr auto known_index_positions = hana::make_range(
						hana::size_c< 0 >, hana::size_c< sizeof...(KD) >);
					return value_type != known_value_type
						|| hana::unpack(known_index_positions,
							[known_indexes, key](auto ... pos){
								return (hana::false_c || ... ||
									(known_indexes[pos].i != key[hana::index_if(
										hana::tuple_c< std::size_t, D ... >,
										detail::equal_to(known_indexes[pos].d)
									).value()]));
							});
				});

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			static_assert(!hana::is_empty(remaining_keys));
#endif

			constexpr auto unknown_indexes = hana::remove_if(
				ds, [](auto d){
					return hana::contains(
						hana::tuple_c< std::size_t, KD ... >, d);
				});

			constexpr auto remaining_indexes =
				[remaining_keys](auto d){
					auto const key_pos =
						hana::index_if(ds, detail::equal_to(d));
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
					static_assert(!hana::is_nothing(key_pos));
#endif
					return hana::unique(hana::transform(
						remaining_keys, [key_pos](auto key){
							return key[key_pos.value()];
						}));
				};

			auto const solve_dimension =
				[remaining_indexes, unknown_indexes](auto pos){
					auto const indexes = remaining_indexes(unknown_indexes[pos]);
					auto const key = unknown_indexes[pos];
					constexpr auto index = indexes[hana::size_c< 0 >];
					return hana::if_(hana::size(indexes) == hana::size_c< 1 >,
						ct_dimension_index< key, index >{}, hana::nothing);
				};

			constexpr auto unknown_index_positions = hana::make_range(
				hana::size_c< 0 >, hana::size(unknown_indexes));
			auto const solved_indexes = hana::unpack(unknown_index_positions,
				[solve_dimension](auto ... pos){
					return hana::remove(
						hana::make_tuple(solve_dimension(pos) ...),
						hana::nothing);
				});
			return hana::unpack(solved_indexes, [](auto ... index){
					return solved_dimensions< sizeof...(index) != 0,
						decltype(index) ... >{};
				});
		}
	};



}


#endif
