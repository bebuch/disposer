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
#include "../tool/depended_t.hpp"

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
#include <boost/hana/all.hpp>

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


	template < std::size_t ... Ds >
	struct solved_dimensions:
		std::conditional_t< (sizeof...(Ds) > 0), hana::true_, hana::false_ >
	{
		static constexpr std::size_t index_count = sizeof...(Ds);

		constexpr solved_dimensions(dimension_index< Ds > ... is)
			: indexes(is ...) {}

		hana::tuple< dimension_index< Ds > ... > indexes;
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
				[](auto i){ return i == hana::size_c< D >; }).value();
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

	/// \brief Tool to evaluate active dimension of inputs if possible
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	class dimension_solver{
	private:
		/// \brief Sorted and summarized dimension numbers
		using numbers = dimension_numbers_t< Ds ... >;

		/// \brief Cartesian product of all dimension numbers
		static constexpr auto indexes = hana::unpack(numbers::values,
			[](auto ... d){
				return hana::cartesian_product(
					hana::make_tuple(hana::make_range(
						hana::size_c< 0 >,
						hana::size(DimensionList::dimensions[d])
					) ...));
			});

		/// \brief Get the value_type of a given numbers index
		template < std::size_t ... Is >
		static constexpr auto value_type_of(
			hana::tuple< hana::size_t< Is > ... > index
		){
			return hana::type_c< Template< typename decltype(
					+DimensionList::dimensions[hana::size_c< Ds >]
						[index[numbers::pos(hana::size_c< Ds >)]]
				)::type ... > >;
		}

		/// \brief hana::true_c if index of dimension D is deducible if indexes
		///        of the dimensions KDs are known, hana::false_c otherwise
		template < std::size_t D, std::size_t ... KDs, typename Indexes >
		static constexpr auto is_deducible_from(Indexes indexes){
			return hana::all(hana::transform(hana::to_tuple(hana::make_range(
					hana::size_c< 0 >,
					hana::size(DimensionList::dimensions[hana::size_c< D >])
				)), [indexes](auto di){
					auto const i_indexes = hana::remove_if(indexes,
						[di](auto index){
							return index[numbers::pos(hana::size_c< D >)] != di;
						});
					if constexpr(sizeof...(KDs) == 0){
						return hana::size_c< 1 > == hana::size(
							hana::unique(hana::transform(i_indexes,
							[](auto index){ return value_type_of(index); })));
					}else{
						return is_deducible_from< KDs ... >(i_indexes);
					}
				}));
		}

		/// \brief hana::true_c if index of dimension D is deducible if indexes
		///        of the dimensions KDs are known, hana::false_c otherwise
		template < std::size_t D, std::size_t ... KDs >
		static constexpr auto is_deducible
			= is_deducible_from< D, KDs ... >(indexes);


		/// \brief Get index of dimension D when dimensions KDs are known
		template < std::size_t D, std::size_t ... KDs >
		static constexpr dimension_index< D > deduce_index(
			hana::size_t< D >,
			type_index const& /*type_index*/,
			hana::tuple< dimension_index< KDs > ... > const& /*known_indexes*/
		){
			return {};
		}

	public:
		/// \brief Get all deducible dimensions when dimension KDs are already
		///        known
		template < std::size_t ... KDs >
		static constexpr auto solve(
			type_index const& type_index,
			hana::tuple< dimension_index< KDs > ... > const& known_indexes
		){
			constexpr auto deducible_dims = hana::remove_if(numbers::values,
				[](auto d){
					constexpr auto kds = hana::tuple_c< std::size_t, KDs ... >;
					return hana::contains(kds, d)
						|| !is_deducible< decltype(d)::value, KDs ... >;
				});

			return hana::unpack(deducible_dims,
				[type_index, known_indexes](auto ... ds){
					return solved_dimensions< decltype(ds)::value ... >(
							deduce_index(ds, type_index, known_indexes) ...
						);
				});
		}
	};



}


#endif
