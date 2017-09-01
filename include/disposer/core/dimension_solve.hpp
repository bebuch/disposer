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

#include <boost/hana/remove_at.hpp>
#include <boost/hana/remove_if.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/all.hpp>


namespace disposer{


	/// \brief Tool to evaluate active dimension of inputs if possible
	template <
		typename DimensionList,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	class dimension_solver{
	private:
		/// \brief Sorted and summarized dimension numbers
		static constexpr auto numbers = dimension_numbers< Ds ... >{};

		/// \brief Converts between packed indexes and corresponding types
		static constexpr auto convert
			= dimension_converter< DimensionList, Template, Ds ... >{};

		/// \brief Use ranges as is since there are no more known dimensions
		template < typename Ranges >
		static auto constexpr sub_ranges_from(Ranges ranges){
			return ranges;
		}

		/// \brief Make a tuple of subranges where every component holds one
		///        index of the known dimension KD
		template < std::size_t KD, std::size_t ... KDs, typename Ranges >
		static auto constexpr sub_ranges_from(Ranges ranges){
			auto const index_pos = numbers.pos(hana::size_c< KD >);
			auto const range = ranges[index_pos];
			auto const ranges_rest = hana::remove_at(ranges, index_pos);
			return hana::unpack(range, [=](auto ... ki){
					return hana::make_tuple(sub_ranges_from< KDs ... >(
						hana::insert(ranges_rest, index_pos,
							hana::make_tuple(ki))) ...);
				});
		}

		/// \brief Create subranges by known dimensions KDs
		template < std::size_t ... KDs >
		static auto constexpr sub_ranges
			= sub_ranges_from< KDs ... >(convert.ranges);

		/// \brief hana::true_c if indexs are deducible by ranges,
		///        hana::false_c otherwise
		template < typename Ranges >
		static constexpr auto is_deducible_from(Ranges ranges){
			auto const indexes = hana::cartesian_product(ranges);
			auto types = hana::transform(indexes,
				[](auto index){
					return convert.value_type_of(index);
				});
			auto const unique_size =
				hana::size(hana::to_set(types));
			return hana::size(types) == unique_size;
		}


		/// \brief hana::true_c if indexs are deducible by ranges if the
		///        dimensions KDs are known, hana::false_c otherwise
		template <
			std::size_t KD,
			std::size_t ... KDs,
			typename Ranges >
		static constexpr auto is_deducible_from(Ranges ranges){
			return hana::all(hana::transform(ranges, [=](auto sub_ranges){
					return is_deducible_from< KDs ... >(sub_ranges);
				}));
		}

		/// \brief hana::true_c if indexs are deducible by ranges if the
		///        dimensions KDs are known, hana::false_c otherwise
		template < std::size_t ... KDs >
		static constexpr auto is_deducible =
			is_deducible_from< KDs ... >(sub_ranges< KDs ... >);


		template < std::size_t D >
		struct index_deducer{
			template < std::size_t ... KDs >
			struct known_dimensions;

			template < std::size_t, std::size_t ... KDs >
			struct reduced_known_dimensions{
				using type = known_dimensions< KDs ... >;
			};

			template < std::size_t, std::size_t ... KDs >
			struct reduced_fn_ptr{
				using type = std::size_t(*)(type_index, decltype(KDs) ...);
			};

			template < std::size_t ... KDs >
			struct known_dimensions{
				/// \brief Get the index of dimension D
				template < typename ... SubRanges >
				static constexpr std::size_t fn(
					type_index ti,
					decltype(KDs) ... kis
				){
					if constexpr(sizeof...(KDs) > 0){
						return [](type_index ti, std::size_t ki, auto ... kis){
								constexpr std::array<
									typename reduced_fn_ptr< KDs ... >::type,
									sizeof...(SubRanges)
								> list{{
									reduced_known_dimensions< KDs ... >::type
										::template fn_address< SubRanges >() ...
								}};

								return list[ki](ti, kis ...);
							}(ti, kis ...);
					}else{
						constexpr std::array<
							std::pair< type_index, std::size_t >,
							sizeof...(SubRanges)
						> types{{
							std::pair(
								type_index::type_id< typename decltype(
										convert.value_type_of(SubRanges{})
									)::type >(),
								decltype(+SubRanges{}[numbers.pos(
									hana::size_c< D >)])::value
							) ...
						}};

						std::size_t i = 0;
						for(auto t: types){
							if(ti == t.first) return t.second;
							++i;
						}

						// TODO: Remove the if as soon as GCC BUG is fixed
						if(i == types.size()){
							throw std::out_of_range(
								"type deduction failed; THIS IS A "
								"SERIOUS DISPOSER BUG!");
						}
						return i;
					}
				}

				/// \brief Get the address the corresponding fn
				template < typename Ranges >
				static constexpr auto fn_address(){
					if constexpr(sizeof...(KDs) > 0){
						return hana::unpack(Ranges{}, [](auto ... sub_ranges){
								return &fn< decltype(sub_ranges) ... >;
							});
					}else{
						auto const indexes = hana::cartesian_product(Ranges{});
						return hana::unpack(indexes, [](auto ... index){
								return &fn< decltype(index) ... >;
							});
					}
				}
			};
		};


		/// \brief A function that returns the index of dimension D after a call
		///        with the type index of the known result type and all known
		///        dimension indexes
		template < std::size_t D, std::size_t ... KDs >
		static constexpr auto index_fn =
			index_deducer< D >::template known_dimensions< KDs ... >
				::template fn_address< decltype(sub_ranges< KDs ... >) >();


		/// \brief Get index of dimension D when dimensions KDs are known
		template <
			std::size_t D,
			std::size_t ... KDs >
		static constexpr index_component< D > deduce_index(
			hana::size_t< D >,
			type_index const& type_index,
			hana::tuple< index_component< KDs > ... > const& known_indexes
		){
			return hana::unpack(known_indexes,
				[&type_index](auto const& ... known_index){
					return index_component< D >{index_fn< D, KDs ... >(
						type_index, known_index.i ...)};
				});
		}

		/// \brief Provided to print a compile time error with the relevant
		///        information
		template < typename Name, std::size_t ... KDs >
		static constexpr void can_not_deduce_dimensions(){
			static_assert(sizeof...(KDs) == 0,
				"Unknown dimensions KDs can't be deduced by input. Please "
				"provide a set_dimension_fn before the input to provide them "
				"manuelly.");
		}


	public:
		/// \brief Constructor
		template < typename ... Dimension >
		constexpr dimension_solver(
			dimension_list< Dimension ... >,
			dimension_referrer< Template, Ds ... >
		) {}

		/// \brief Get all deducible dimensions when dimension KDs are already
		///        known
		template < typename Name, bool ... KDs >
		static constexpr auto solve(
			Name const&,
			type_index const& type_index,
			partial_deduced_list_index< KDs ... > const& known_indexes
		){
			auto const kds = hana::transform(hana::filter(
				hana::zip(known_indexes.ds, known_indexes.index),
				[](auto const& pair){
					auto const known = pair[hana::size_c< 1 >] == hana::nothing;
					if constexpr(known){
						return hana::false_c;
					}else{
						return hana::contains(numbers.numbers,
							pair[hana::size_c< 0 >]);
					}
				}), [](auto const& pair){
					auto const d = pair[hana::size_c< 0 >];
					return index_component< std::size_t(d) >{
						pair[hana::size_c< 1 >].value()};
				});

			auto const known_dims = hana::transform(kds,
				[](auto kd){ return hana::size_c< kd.d >; });

			auto const unknown_dims = hana::filter(numbers.packed,
				[known_dims](auto d){ return !hana::contains(known_dims, d); });

			auto const has_unknown_dims = !hana::is_empty(unknown_dims);

			if constexpr(!has_unknown_dims){
				return solved_dimensions{};
			}else{
				auto const is_deducible_c = hana::unpack(known_dims,
					[](auto ... kd){
						return is_deducible< std::size_t(kd) ... >;
					});

				if constexpr(is_deducible_c){
					return hana::unpack(unknown_dims,
						[type_index, &kds](auto ... ds){
							return solved_dimensions{
									deduce_index(ds, type_index, kds) ...
								};
						});
				}else{
					hana::unpack(unknown_dims, [](auto ... ds){
							can_not_deduce_dimensions
								< Name, std::size_t(ds) ... >();
						});
				}
			}
		}
	};


	template <
		typename ... Dimension,
		template < typename ... > typename Template,
		std::size_t ... Ds >
	dimension_solver(
		dimension_list< Dimension ... >,
		dimension_referrer< Template, Ds ... >
	) -> dimension_solver< dimension_list< Dimension ... >, Template, Ds ... >;


}


#endif