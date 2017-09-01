//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_input__hpp_INCLUDED_
#define _disposer__core__make_input__hpp_INCLUDED_

#include "input_name.hpp"
#include "dimension_converter.hpp"


namespace disposer{


	inline output_base* get_output_ptr(
		input_list const& inputs,
		std::string_view const& name
	){
		auto const iter = inputs.find(name);
		return iter != inputs.end() ? iter->second : nullptr;
	}


	template < typename Name, typename Type, bool IsRequired >
	struct input_construct_data{
		static constexpr auto log_name = "input"sv;

		input_construct_data(output_base* const output)noexcept
			: output(output){}

		output_base* const output;
	};


	template < typename Name, bool IsRequired, typename ... Ts >
	constexpr auto input_variant_type(hana::tuple< Ts ... >)noexcept{
		if constexpr(IsRequired){
			return hana::type_c< std::variant< input_construct_data<
				Name, typename Ts::type, IsRequired > ... > >;
		}else{
			return hana::type_c< std::variant<
				hana::false_, input_construct_data<
					Name, typename Ts::type, IsRequired > ... > >;
		}
	}

	template < typename Name, bool IsRequired, typename Types >
	using input_variant = typename
		decltype(input_variant_type< Name, IsRequired >(Types{}))::type;

	template <
		typename Name,
		typename DimensionConverter,
		bool IsRequired,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto make_data(
		input_maker< Name, DimensionConverter, IsRequired >,
		dimension_list< Ds ... >,
		module_make_data const& data,
		partial_deduced_list_index< KDs ... > const& old_dims,
		hana::tuple< Ts ... >&& previous_makers
	){
		using result_type =
			input_variant< Name, IsRequired, DimensionConverter::types >;

		auto const output_ptr = get_output_ptr(data.inputs,
			to_std_string_view(Name));

		if(IsRequired || output_ptr != nullptr){
			auto const iter = DimensionConverter::type_indexes.find(
				output_ptr->type());
			if(iter == DimensionConverter::type_indexes.end()){
				throw std::logic_error("Type of connected output which is ["
					+ iter->pretty_name() + "] is not compatible with input");
			}
		}

		auto const dims = [&old_dims](){
				if constexpr(IsRequired){
					constexpr dimension_solver solver(
						dimension_list< Ds ... >{}, DimensionConverter{});
					return partial_deduced_list_index(old_dims,
						solver::solve(Name{}, output_ptr->type(), old_dims));
				}else{
					return old_dims;
				}
			}();

		if constexpr(!IsRequired){
			if(output_ptr == nullptr) return result_type();
		}

		constexpr auto type_to_data = hana::unpack(DimensionConverter::types,
			[](auto ... t){
				return std::unordered_map{{
					type_index::type_id(type),
					[](output_base* output_ptr){
						return input_construct_data< Name, typename
							decltype(t)::type, IsRequired >{output_ptr};
					}} ...};
			});

		return hana::concat(std::move(previous_makers), hana::make_pair(dims,
			result_type(type_to_data[output_ptr->type()](output_ptr))));
	}



	/// \brief Provids types for constructing an input
	template <
		typename Name,
		typename DimensionConverter,
		bool IsRequired >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;
	};


	/// \brief Configuration class for inputs
	template < bool IsRequired >
	struct is_required: std::bool_constant< IsRequired >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto required = is_required< true >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto not_required = is_required< false >{};


	/// \brief Creates a \ref input_maker object
	template <
		char ... C
		template < typename ... > typename Template,
		std::size_t ... D,
		bool IsRequired = true >
	constexpr auto make(
		input_name< C ... > const&,
		dimension_converter< Template, D ... > const&,
		is_required< IsRequired > = required
	){
		return input_maker<
			input_name< C ... >,
			dimension_converter< Template, D ... >,
			IsRequired >{};
	}


	/// \brief Make data for a required input with known type
	template < typename Name, typename T >
	struct final_input_make_data{
		output_make_data* output;

		template < std::size_t D >
		get_dimension()
	};



}


#endif
