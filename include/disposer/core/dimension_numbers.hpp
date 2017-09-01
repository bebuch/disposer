//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension_numbers__hpp_INCLUDED_
#define _disposer__core__dimension_numbers__hpp_INCLUDED_

#include "dimension.hpp"

#include <boost/hana/range.hpp>
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/hana/unique.hpp>
#include <boost/hana/zip.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/replicate.hpp>
#include <boost/hana/not_equal.hpp>

#include <unordered_set>


namespace disposer{


	/// \brief Converts between a list of dimension numbers an its packed index
	///        correspondence
	template < std::size_t ... Ds >
	struct dimension_numbers{
		/// \brief All dimension numbers in the defined order
		static constexpr auto numbers = hana::tuple_c< std::size_t, Ds ... >;

		/// \brief Count of dimensions
		static constexpr auto numbers_count = sizeof...(Ds);

		/// \brief Dimension number in sorted order and without duplicates
		static constexpr auto packed = hana::unpack(
			hana::unique(hana::sort(numbers)),
			[](auto ... d){ return hana::make_tuple(d ...); });

		/// \brief Count of dimensions in packed
		static constexpr auto packed_count = hana::size(packed).value;


		/// \brief Position of a dimension number D in packed
		template < std::size_t D >
		static constexpr auto pos(hana::size_t< D >){
			return hana::index_if(packed,
				hana::equal.to(hana::size_c< D >)).value();
		}
	};

	/// \brief Pair of a dimension number and the index of its active type
	template < std::size_t D >
	struct index_component{
		/// \brief Dimension number
		static constexpr std::size_t d = D;

		/// \brief Index of the active type of the dimension
		std::size_t i;
	};


	/// \brief Contains deduced dimension index components, derived from
	///        hana::true_ if at least one dimension is deduced, otherwise
	///        hana::false_
	template < std::size_t ... Ds >
	struct solved_dimensions:
		std::conditional_t< (sizeof...(Ds) > 0), hana::true_, hana::false_ >
	{
		/// \brief Count of deduced dimension index components
		static constexpr std::size_t index_count = sizeof...(Ds);

		/// \brief Constructor
		constexpr solved_dimensions(index_component< Ds > ... is)
			: indexes(is ...) {}

		/// \brief Tuple of the deduced index components
		hana::tuple< index_component< Ds > ... > indexes;
	};

	/// \brief Optional index components in the same order as in a DimensionList
	template < bool ... DKs >
	struct partial_deduced_list_index{
		/// \brief List which says if a dimension is known
		static constexpr auto is_dim_known = hana::tuple_c< bool, DKs ... >;

		/// \brief All dimension numbers
		static constexpr auto ds =
			hana::to_tuple(hana::range_c< std::size_t, 0, sizeof...(DKs) >);

		/// \brief List of known dimensions
		static constexpr auto known_dims =
			hana::filter(ds, [](auto d){ return is_dim_known[d]; });

		/// \brief List of unknown dimensions
		static constexpr auto unknown_dims =
			hana::filter(ds, [](auto d){ return !is_dim_known[d]; });


		/// \brief List of indexes
		hana::tuple< std::conditional_t< DKs,
			hana::optional< std::size_t >, hana::optional<> > ... > index;


		/// \brief Construct without any known dimensions
		constexpr partial_deduced_list_index(): index{} {}

		/// \brief Helper constructor
		template < std::size_t ... KDs, bool ... ODKs, std::size_t ... Ds >
		constexpr partial_deduced_list_index(
			std::index_sequence< KDs ... >,
			partial_deduced_list_index< ODKs ... > const& old,
			solved_dimensions< Ds ... > const& solved
		): index{
				[](auto const& old, auto const& solved, auto i, auto known){
					if constexpr(known){
						return old.index[i];
					}else{
						// return the deduced dimension or hana::nothing
						auto optional =
							hana::find_if(solved.indexes, [](auto const& v){
								return hana::typeid_(v) == hana::type_c<
									index_component< decltype(i)::value > >;
							});
						if constexpr(optional == hana::nothing){
							return hana::nothing;
						}else{
							return hana::just(optional.value().i);
						}
					}
				}(old, solved, hana::size_c< KDs >, hana::bool_c< ODKs >) ...
			}
		{
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			static_assert(
				sizeof...(KDs) == sizeof...(ODKs));
			static_assert(hana::all_of(hana::tuple_c< std::size_t, Ds ... >,
				hana::less.than(hana::size_c< sizeof...(KDs) >)));

			static_assert(hana::all_of(hana::tuple_c< std::size_t, Ds ... >,
				[](auto ds){ return !hana::tuple_c< bool, ODKs ... >[ds]; }));
#endif
		}
	};


	template < std::size_t ... KDs, bool ... ODKs, std::size_t ... Ds >
	partial_deduced_list_index(
		std::index_sequence< KDs ... >,
		partial_deduced_list_index< ODKs ... > const& old,
		solved_dimensions< Ds ... > const& solved
	) -> partial_deduced_list_index<
			(ODKs || hana::contains(
				hana::tuple_c< std::size_t, Ds ... >, hana::size_c< KDs >)) ...
		>;

	/// \brief Constructs a partial_deduced_list_index with DimensionCount
	///        unknown index components
	template < std::size_t DimensionCount >
	constexpr auto undeduced_list_index_c = hana::unpack(
		hana::replicate< hana::tuple_tag >(
			hana::false_c, hana::size_c< DimensionCount >
		), [](auto ... false_){
			return partial_deduced_list_index< decltype(false_)::value ... >{};
		});

	/// \brief Creates a partial_deduced_list_index for unit tests
	template < std::size_t DimensionCount, std::size_t ... KDs >
	constexpr auto make_list_index(index_component< KDs > ... is){
		return partial_deduced_list_index{
			std::make_index_sequence< DimensionCount >(),
			undeduced_list_index_c< DimensionCount >,
			solved_dimensions{is ...}};
	}

	/// \brief Index components in the same order as dimension_numbers::packed
	///        dimensions
	template < std::size_t PackedDimCount >
	struct packed_index: std::array< std::size_t, PackedDimCount >{
		/// \brief Get index component number D from list_index if it is already
		///        deduced, static_assert otherwise
		template < bool ... DKs, std::size_t D >
		static constexpr std::size_t get(
			partial_deduced_list_index< DKs ... > const& list_index,
			hana::size_t< D > d
		){
			auto const& i = list_index.index[d];
			auto const is_deduced = i != hana::nothing;
			static_assert(is_deduced, "dimension number D is not deduced yet");
			return i.value();
		}

		/// \brief Construction by a undeduced_list_index object
		///
		/// Requirement:  Ds must be sorted and unique
		template < bool ... DKs, std::size_t ... Ds >
		constexpr packed_index(
			partial_deduced_list_index< DKs ... > list_index,
			hana::tuple< hana::size_t< Ds > ... >)
			: std::array< std::size_t, PackedDimCount >{{
					get(list_index, hana::size_c< Ds >) ...
				}} {}

		/// \brief Construction by a indexes
		///
		/// Requirement:  Ds must be sorted and unique
		template < std::size_t ... Is >
		constexpr packed_index(hana::tuple< hana::size_t< Is > ... >)
			: std::array< std::size_t, PackedDimCount >{{Is ...}} {}
	};

	/// \brief Template duduction guide
	template < bool ... DKs, std::size_t ... Ds >
	packed_index(partial_deduced_list_index< DKs ... >,
		hana::tuple< hana::size_t< Ds > ... >
	) -> packed_index< sizeof...(Ds) >;

	template < std::size_t ... Is >
	packed_index(hana::tuple< hana::size_t< Is > ... >)
		-> packed_index< sizeof...(Is) >;


	/// \brief Converts between packed indexes and corresponding types
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	struct dimension_converter{
		/// \brief Sorted and summarized dimension numbers
		static constexpr auto numbers = dimension_numbers< Ds ... >{};


		/// \brief Get the value_type of a given numbers index
		template < std::size_t ... Is >
		static constexpr auto value_type_of(
			hana::tuple< hana::size_t< Is > ... > index
		)noexcept{
			return hana::type_c< Template< typename decltype(
					+DimensionList::dimensions[hana::size_c< Ds >]
						[index[numbers.pos(hana::size_c< Ds >)]]
				)::type ... > >;
		}

		/// \brief Tuple of ranges of all indexes
		static constexpr auto ranges = hana::unpack(numbers.packed,
			[](auto ... d){
				return hana::make_tuple(hana::to_tuple(hana::make_range(
					hana::size_c< 0 >,
					hana::size(DimensionList::dimensions[d])
				)) ...);
			});

		/// \brief Unique list of all possible types
		static constexpr auto types =
			hana::to_tuple(hana::to_set(hana::transform(
				hana::cartesian_product(ranges),
				[](auto index){ return value_type_of(index); })));

		/// \brief Unique list of all possible type indexes
		static std::unordered_set< type_index > const type_indexes;

		/// \brief Map from packed index to index in types
		static std::map<
			packed_index< dimension_numbers< Ds ... >::packed_count >,
			type_index > const packed_index_to_type_index;
	};

	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	std::unordered_set< type_index > const
		dimension_converter< DimensionList, Template, Ds ... >::type_indexes =
			hana::unpack(types, [](auto ... type){
					return std::unordered_set{
						type_index::type_id< typename decltype(type)::type >()
						...};
				});

	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	std::map<
			packed_index< dimension_numbers< Ds ... >::packed_count >,
			type_index
		> const dimension_converter< DimensionList, Template, Ds ... >
		::packed_index_to_type_index =
			hana::unpack(hana::transform(hana::cartesian_product(ranges),
				[](auto index){
					return hana::make_pair(index, type_index::type_id<
						typename decltype(value_type_of(index))::type >());
				}), [](auto&& ... entry){
					return std::map< packed_index<
							dimension_numbers< Ds ... >::packed_count >,
							type_index
						>{{hana::first(entry), hana::second(entry)} ...};
				});



}


#endif
