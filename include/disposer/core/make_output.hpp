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

#include "input_name.hpp"
#include "dimension_converter.hpp"


namespace disposer{


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
	auto make_data(
		output_maker< Name, DimensionConverter >,
		dimension_list< Ds ... >,
		module_make_data const& data,
		partial_deduced_list_index< KDs ... > const& dims,
		hana::tuple< Ts ... >&& previous_makers
	){
		using result_type = output_variant< Name, DimensionConverter::types >;

		auto const active_type =
			DimensionConverter::packed_index_to_type_index.at(dims);

		auto const use_count = get_use_count(data.outputs,
			to_std_string_view(Name));

		constexpr auto type_to_data = hana::unpack(DimensionConverter::types,
			[](auto ... t){
				return std::unordered_map{{
					type_index::type_id(type),
					[](std::size_t use_count){
						return output_construct_data< Name, typename
							decltype(t)::type >{use_count};
					}} ...};
			});

		return hana::concat(std::move(previous_makers), hana::make_pair(dims,
			result_type(type_to_data[active_type](use_count))));
	}


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
		dimension_converter< Template, D ... > const&
	){
		return output_maker<
			output_name< C ... >,
			dimension_converter< Template, D ... > >{};
	}


}


#endif
