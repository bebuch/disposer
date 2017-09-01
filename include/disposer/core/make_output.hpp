//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_output__hpp_INCLUDED_
#define _disposer__core__make_output__hpp_INCLUDED_

#include "output_name.hpp"
#include "dimension_referrer.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/to_std_string_view.hpp"

#include <variant>
#include <unordered_map>


namespace disposer{


	using namespace std::literals::string_view_literals;


	/// \brief Provid types for constructing an output
	template <
		typename Name,
		typename DimensionConverter >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;
	};


	/// \brief Creates a \ref output_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D >
	constexpr auto make(
		output_name< C ... > const&,
		dimension_referrer< Template, D ... > const&
	){
		return output_maker<
			output_name< C ... >,
			dimension_referrer< Template, D ... > >{};
	}


	inline std::size_t get_use_count(
		output_list const& outputs,
		std::string_view const& name
	){
		auto const iter = outputs.find(name);
		return iter != outputs.end() ? iter->second : 0;
	}


	template < typename Name, typename Type >
	struct output_construct_data{
		static constexpr auto log_name = "output"sv;

		output_construct_data(std::size_t use_count)noexcept
			: use_count(use_count) {}

		std::size_t const use_count;
	};

	template < typename Name, typename ... Ts >
	constexpr auto output_variant_type(hana::tuple< Ts ... >)noexcept{
		return hana::type_c< std::variant< output_construct_data<
					Name, typename Ts::type > ... > >;
	}

	template < typename Name, typename Types >
	using output_variant = typename
		decltype(output_variant_type< Name >(Types{}))::type;

	template <
		typename Name,
		typename DimensionConverter,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto make_construct_data(
		output_maker< Name, DimensionConverter >,
		dimension_list< Ds ... >,
		module_make_data const& data,
		partial_deduced_list_index< KDs ... > const& dims,
		hana::tuple< Ts ... >&& previous_makers
	){
		auto constexpr converter =
			DimensionConverter::template convert< dimension_list< Ds ... > >;
		using result_type = output_variant< Name, decltype(converter.types) >;

		auto const active_type = converter.packed_index_to_type_index.at(
			packed_index{dims, converter.numbers.numbers});

		auto const use_count = get_use_count(data.outputs,
			detail::to_std_string_view(Name{}));

		auto const make_fn = [](auto t){
				return [](std::size_t use_count)->result_type{
					return output_construct_data< Name, typename
						decltype(t)::type >{use_count};
				};
			};

		auto const type_to_data = hana::unpack(converter.types,
			[make_fn](auto ... t){
				return std::unordered_map<
						type_index, result_type(*)(std::size_t)
					>{{
						type_index::type_id< typename decltype(t)::type >(),
						make_fn(t)
					} ...};
			});

		return hana::append(std::move(previous_makers), hana::make_pair(dims,
			type_to_data.at(active_type)(use_count)));
	}


}


#endif
