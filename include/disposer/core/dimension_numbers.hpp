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

#include "../tool/type_index.hpp"

#include <boost/hana/range.hpp>
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/hana/unique.hpp>
#include <boost/hana/zip.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/replicate.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/remove_at.hpp>

#include <unordered_set>


namespace disposer{


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
	template < std::size_t ... SDs >
	struct solved_dimensions:
		std::conditional_t< (sizeof...(SDs) > 0), hana::true_, hana::false_ >
	{
		/// \brief Count of deduced dimension index components
		static constexpr std::size_t index_count = sizeof...(SDs);

		/// \brief Get the numbers of all solved dimensions
		static constexpr auto dimension_numbers()noexcept{
			return hana::tuple_c< std::size_t, SDs ... >;
		}


		/// \brief Constructor
		constexpr solved_dimensions(index_component< SDs > ... is)
			: indexes(is ...) {}

		/// \brief Construct by tuple
		constexpr solved_dimensions(
			hana::tuple< index_component< SDs > ... >&& idx
		): indexes(idx) {}

		/// \brief Tuple of the deduced index components
		hana::tuple< index_component< SDs > ... > indexes;

		/// \brief Get the solved dimensions without the first dimension
		constexpr auto rest()const noexcept{
			return disposer::solved_dimensions{
				hana::remove_at(indexes, hana::size_c< 0 >)};
		}

		/// \brief Get the number if the first solved dimensions
		static constexpr auto dimension_number()noexcept{
			return dimension_numbers()[hana::size_c< 0 >];
		}

		/// \brief Get index if the first dimension
		constexpr std::size_t index_number()const noexcept{
			return indexes[hana::size_c< 0 >].i;
		}

		/// \brief true if indexes has no elements, false otherwise
		static constexpr std::size_t is_empty()noexcept{
			return index_count == 0;
		}
	};


	template < typename T >
	struct is_solved_dimensions: hana::false_{};

	template < std::size_t ... SDs >
	struct is_solved_dimensions< solved_dimensions< SDs ... > >: hana::true_{};

	template < typename T >
	constexpr auto is_solved_dimensions_v = is_solved_dimensions< T >::value;


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
					if constexpr(constexpr std::size_t k = known; k){
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
				}} {
					(void)list_index; // Silance GCC
				}

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


	/// \brief Make tuple of ranges of all indexes
	template < typename ... Dimensions, std::size_t ... Ds >
	constexpr auto make_ranges(
		dimension_list< Dimensions ... >,
		hana::tuple< hana::size_t< Ds > ... > const& numbers
	)noexcept{
		return hana::unpack(numbers,
			[](auto ... d){
				return hana::make_tuple(hana::to_tuple(hana::make_range(
					hana::size_c< 0 >,
					hana::size(dimension_list< Dimensions ... >::dimensions[d])
				)) ...);
			});
	}

	/// \brief Converts between packed indexes and corresponding types
	template <
		typename DimensionList,
		std::size_t ... Ds >
	struct dimension_ranges: dimension_numbers< Ds ... >{
		using dimension_numbers< Ds ... >::packed;

		/// \brief Tuple of ranges of all indexes
		static constexpr auto ranges = make_ranges(DimensionList{}, packed);

		/// \brief Tuple of all indexes
		static constexpr auto indexes = []{
				if constexpr(hana::is_empty(ranges)){
					return hana::make_tuple(hana::make_tuple());
				}else{
					return hana::cartesian_product(ranges);
				}
			}();
	};


	/// \brief Converts between packed indexes and corresponding types
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	struct dimension_converter: dimension_ranges< DimensionList, Ds ... >{
		using base = dimension_ranges< DimensionList, Ds ... >;
		using base::packed_count;
		using base::indexes;
		using base::pos;

		/// \brief Get the value_type of a given numbers index
		template < std::size_t ... Is >
		static constexpr auto value_type_of(
			hana::tuple< hana::size_t< Is > ... > index
		)noexcept{
			return hana::type_c< Template< typename decltype(
					+DimensionList::dimensions[hana::size_c< Ds >]
						[index[pos(hana::size_c< Ds >)]]
				)::type ... > >;
		}

		/// \brief Unique list of all possible types
		static constexpr auto types =
			hana::to_tuple(hana::to_set(hana::transform(indexes,
				[](auto index){ return value_type_of(index); })));

		/// \brief Get a list of all valid type indexes
		static auto get_type_indexes(){
			return hana::unpack(types, [](auto ... type){
					return std::unordered_set< type_index >{
						type_index::type_id<
							typename decltype(type)::type >() ...};
				});
		}

		/// \brief true if type index belongs to one of the types,
		///        false otherwise
		static bool is_valid(type_index const& ti){
			static auto const type_indexes = get_type_indexes();
			return type_indexes.find(ti) != type_indexes.end();
		}

		/// \brief Get the type index of the given packed index,
		///        throws if index dosn't exist
		///
		/// TODO: Don't throw, just crash. Wrong index should never appear.
		static type_index to_type_index(
			packed_index< packed_count > const& index
		){
			static auto const map =
				hana::unpack(hana::transform(indexes,
					[](auto index){
						return hana::make_pair(index,
							type_index::type_id< typename
							decltype(value_type_of(index))::type >());
					}), [](auto&& ... entry){
						return std::map<
								packed_index< packed_count >,
								type_index >
							{{hana::first(entry), hana::second(entry)} ...};
					});

			return map.at(index);
		}
	};


}


#endif
