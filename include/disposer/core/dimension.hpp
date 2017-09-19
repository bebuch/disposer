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

		static_assert(type_count > 0,
			"dimension's must contain at least 1 type");

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


		/// \brief Default constructor
		///
		/// Disabled if Ts is empty to resolve ambiguity.
		template < typename Dummy = void, typename = std::enable_if_t<
			(sizeof...(Ts) > 0), Dummy > >
		constexpr dimension()noexcept{}

		/// \brief Construct by hana::type's
		constexpr dimension(hana::basic_type< Ts > ...)noexcept{}
	};


	/// \brief Creates a dimension object
	template < typename ... Ts >
	constexpr dimension< Ts ... > dimension_c{};


	/// \brief List of type lists of a module
	template < typename ... Ds >
	struct dimension_list{
		static_assert(hana::all_of(
			hana::make_tuple(Ds{} ...), hana::is_a< dimension_tag >),
			"Ds must be a disposer::dimension< ... >");

		/// \brief Tuple of type lists, Ds is always a \ref dimension
		static constexpr auto dimensions = hana::make_tuple(Ds::types ...);

		/// \brief Default constructor
		///
		/// Disabled if Ds is empty to resolve ambiguity.
		template < typename Dummy = void, typename = std::enable_if_t<
			(sizeof...(Ds) > 0), Dummy > >
		constexpr dimension_list()noexcept{}

		/// \brief Constructor
		constexpr dimension_list(Ds const& ...)noexcept{}
	};


	/// \brief Pair of a dimension number and the index of its active type
	template < std::size_t DI, std::size_t I >
	struct ct_index_component{
		/// \brief Dimension number
		static constexpr auto d = hana::size_c< DI >;

		/// \brief Index of the active type of the dimension
		static constexpr auto i = hana::size_c< I >;
	};


	/// \brief Create a new dimension list, where the given dimension has
	///        the indexed type
	template < typename ... Ds, std::size_t DI, std::size_t I >
	constexpr auto reduce_dimension_list(
		dimension_list< Ds ... >,
		ct_index_component< DI, I >
	)noexcept{
		return hana::unpack(hana::range_c< std::size_t, 0, sizeof...(Ds) >,
			[](auto ... n){
				constexpr auto calc = [](auto n, auto dim)noexcept{
						if constexpr(n.value == DI){
							return dimension{dim.types[hana::size_c< I >]};
						}else{
							return dim;
						}
					};

				return dimension_list{calc(n, Ds{}) ...};
			});
	}


}


#endif
