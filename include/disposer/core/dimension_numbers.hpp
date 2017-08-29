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
#include <boost/hana/sort.hpp>


namespace disposer{


	/// \brief Pair of a dimension number and the index of its active type
	template < std::size_t D >
	struct dimension_index{
		/// \brief Dimension number
		static constexpr std::size_t d = D;

		/// \brief Index of the active type of the dimension
		std::size_t i;
	};


	template < std::size_t ... Ds >
	struct solved_dimensions:
		std::conditional_t< (sizeof...(Ds) > 0), hana::true_, hana::false_ >
	{
		static constexpr std::size_t index_count = sizeof...(Ds);

		constexpr solved_dimensions(dimension_index< Ds > ... is)
			: indexes(is ...) {}

		hana::tuple< dimension_index< Ds > ... > indexes;
	};

	/// \brief List of optional_dimension_index'es
	template < std::size_t DimensionCount >
	struct optional_dimension_indexes
		: std::array< std::optional< std::size_t >, DimensionCount >{};

	/// \brief List of optional_dimension_index'es
	template < std::size_t DimensionCount >
	struct rt_dimension_index: std::array< std::size_t, DimensionCount >{
		template < std::size_t DLCount, std::size_t ... Ds >
		rt_dimension_index(
			optional_dimension_indexes< DLCount > index,
			hana::tuple< hana::size_t< Ds > ... >)
			// TODO: index has all indexes from the list, we need only the
			//       indexes Ds in the order of dimension_numbers
	};


	/// \brief List of dimension numbers
	template < std::size_t ... Ds >
	struct dimension_numbers{
		/// \brief List of dimension numbers
		static constexpr auto values = hana::tuple_c< std::size_t, Ds ... >;

		/// \brief Position of a dimension number in values
		template < std::size_t D >
		static constexpr auto pos(hana::size_t< D >){
			return hana::index_if(values,
				hana::equal.to(hana::size_c< D >)).value();
		}
	};

	/// \brief A dimension_numbers object with sorted and
	///        summarized dimension numbers
	template < std::size_t ... Ds >
	constexpr auto dimension_numbers_c = hana::unpack(
		hana::unique(hana::sort(hana::tuple_c< std::size_t, Ds ... >)),
		[](auto ... d){
			return dimension_numbers< decltype(d)::value ... >{};
		});

	/// \brief Get dimension_numbers with sorted and
	///        summarized dimension numbers
	template < std::size_t ... Ds >
	using dimension_numbers_t =
		decltype(dimension_numbers_c< Ds ... >);


	/// \brief Converts between dimension_indexes and corresponding types
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	struct dimension_converter{
		/// \brief Count of dimensions in the dimension list
		static constexpr auto dimension_count
			= hana::size(DimensionList::dimensions);

		/// \brief Sorted and summarized dimension numbers
		using numbers = dimension_numbers_t< Ds ... >;

		/// \brief Get the value_type by runtime index
		static constexpr auto value_type_of(
			optional_dimension_indexes< dimension_count > index
		)noexcept{

		}

		/// \brief Get the value_type of a given numbers index
		template < std::size_t ... Is >
		static constexpr auto value_type_of(
			hana::tuple< hana::size_t< Is > ... > index
		)noexcept{
			return hana::type_c< Template< typename decltype(
					+DimensionList::dimensions[hana::size_c< Ds >]
						[index[numbers::pos(hana::size_c< Ds >)]]
				)::type ... > >;
		}

		/// \brief Tuple of ranges of all indexes
		static constexpr auto ranges = hana::unpack(numbers::values,
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

		/// \brief Map from numbers index index to type
		static constexpr auto index_to_type =
			hana::unpack(hana::transform(hana::cartesian_product(ranges),
				[](auto index){
					return hana::make_pair(index, hana::index_if(types,
						hana::equal.to(value_type_of(index)))).value();
				}), [](auto&& ... entry){
					return std::map< , std::size_t >{
						{hana::first(entry), hana::second(entry)} ...};
				});
	};





}


#endif
