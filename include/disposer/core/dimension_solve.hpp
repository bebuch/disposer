//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__dimension_solve__hpp_INCLUDED_
#define _disposer__core__dimension_solve__hpp_INCLUDED_

#include "dimension_referrer.hpp"

#include "../tool/type_index.hpp"
#include "../tool/type_list_as_string.hpp"
#include "../tool/to_std_string.hpp"

#include <boost/hana/remove_at.hpp>
#include <boost/hana/remove_if.hpp>
#include <boost/hana/not_equal.hpp>


namespace disposer{


	/// \brief Tool to evaluate active dimension of inputs if possible
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... DIs >
	class dimension_solver
		: public dimension_converter< DimensionList, Template, DIs ... >{
	private:
		using base = dimension_converter< DimensionList, Template, DIs ... >;
		using base::indexes;
		using base::value_type_of;
		using base::get_type_indexes;
		using base::packed;
		using base::pos;

		/// \brief hana::true_c if indexes are deducible by ranges,
		///        hana::false_c otherwise
		static constexpr auto is_deducible = []{
				auto types = hana::transform(indexes,
					[](auto index){ return value_type_of(index); });
				auto const unique_size = hana::size(hana::to_set(types));
				return hana::size(types) == unique_size;
			}();

		/// \brief Provided to print a compile time error with the relevant
		///        information
		template < typename Name, std::size_t ... KDIs >
		static constexpr void can_not_deduce_dimensions(){
			static_assert(sizeof...(KDIs) == 0,
				"Unknown dimensions KDIs can't be deduced by input. Please "
				"provide a set_dimension_fn before the input to provide them "
				"manuelly.");
		}


	public:
		/// \brief Constructor
		template < typename ... Dimension >
		constexpr dimension_solver(
			dimension_list< Dimension ... >,
			dimension_referrer< Template, DIs ... >
		) {}

		/// \brief Get all deducible dimensions when dimension KDIs are already
		///        known
		template < typename Name >
		static auto solve(
			Name const&,
			type_index const& ti
		){
			constexpr auto unknown_dim_indexes = hana::remove_if(packed,
				[](auto di){
					return hana::size(DimensionList::dimensions[di])
						== hana::size_c< 1 >;
				});

			if constexpr(hana::size(unknown_dim_indexes) == hana::size_c< 0 >){
				return solved_dimensions{};
			}else if constexpr(is_deducible){
				return hana::unpack(unknown_dim_indexes, [&ti](auto ... udis){
					using solved_type =
						solved_dimensions< std::size_t(udis) ... >;

					auto const calc_idx = [udis ...](auto idx){
							(void)idx; // Silance GCC if udis is empty
							return solved_type{
								index_component< std::size_t(decltype(udis){}) >
									{idx[pos(udis)]} ...};
						};

					auto const types = hana::unpack(indexes,
						[calc_idx](auto ... index){
							return std::array<
								std::pair< type_index, solved_type >,
								sizeof...(index)
							>{{
								std::pair(
									type_index::type_id< typename
										decltype(value_type_of(index))::type
									>(),
									calc_idx(index)
								) ...
							}};
						});

					for(auto t: types){
						if(ti == t.first) return t.second;
					}

					throw std::logic_error(
						"type of connected output which is ["
						+ ti.pretty_name()
						+ "] is not compatible with input("
						+ detail::to_std_string(Name{})
						+ "), valid types are: "
						+ type_list_as_string(get_type_indexes()));
				});
			}else{
				hana::unpack(unknown_dim_indexes, [](auto ... udis){
						can_not_deduce_dimensions
							< Name, std::size_t(udis) ... >();
					});
			}
		}
	};


	template <
		typename ... Dimension,
		template < typename ... > typename Template,
		std::size_t ... DIs >
	dimension_solver(
		dimension_list< Dimension ... >,
		dimension_referrer< Template, DIs ... >
	) -> dimension_solver< dimension_list< Dimension ... >, Template, DIs ... >;


}


#endif
