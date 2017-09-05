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
#include "dimension_solve.hpp"
#include "output_base.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/to_std_string_view.hpp"
#include "../tool/type_list_as_string.hpp"

#include <variant>
#include <unordered_map>


namespace disposer{


	using namespace std::literals::string_view_literals;


	/// \brief Provids types for constructing an input
	template <
		typename Name,
		typename DimensionReferrer,
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
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		bool IsRequired = true >
	constexpr auto make(
		input_name< C ... > const&,
		dimension_referrer< Template, D ... > const&,
		is_required< IsRequired > = required
	){
		return input_maker<
			input_name< C ... >,
			dimension_referrer< Template, D ... >,
			IsRequired >{};
	}


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
		return hana::type_c< std::variant< input_construct_data<
			Name, typename Ts::type, IsRequired > ... > >;
	}

	template < typename Name, bool IsRequired, typename Types >
	using input_variant = typename
		decltype(input_variant_type< Name, IsRequired >(Types{}))::type;

	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto make_construct_data(
		input_maker< Name, DimensionReferrer, IsRequired >,
		dimension_list< Ds ... >,
		module_make_data const& data,
		partial_deduced_list_index< KDs ... > const& old_dims,
		hana::tuple< Ts ... >&& previous_makers
	){
		auto constexpr converter =
			DimensionReferrer::template convert< dimension_list< Ds ... > >;
		using result_type =
			input_variant< Name, IsRequired, decltype(converter.types) >;

		auto const output_ptr = get_output_ptr(data.inputs,
			detail::to_std_string_view(Name{}));

		if constexpr(IsRequired){
			if(output_ptr == nullptr){
				throw std::logic_error("input is required but not set");
			}
		}

		auto const dims = [&old_dims, output_ptr](){
				if constexpr(IsRequired){
					constexpr dimension_solver solver(
						dimension_list< Ds ... >{}, DimensionReferrer{});
					return partial_deduced_list_index(
						std::make_index_sequence< sizeof...(KDs) >(),
						old_dims, solver.solve(
							Name{}, output_ptr->get_type(), old_dims));
				}else{
					(void)output_ptr; // Silance GCC
					return old_dims;
				}
			}();

		if(IsRequired || output_ptr != nullptr){
			auto const type = output_ptr->get_type();
			if(
				auto const iter = converter.type_indexes.find(type);
				iter == converter.type_indexes.end()
			){
				throw std::logic_error("type of connected output which is ["
					+ type.pretty_name()
					+ "] is not compatible with input, valid types are: "
					+ type_list_as_string(converter.type_indexes));
			}

			auto const active_type = converter.packed_index_to_type_index.at(
				packed_index{dims, converter.numbers.packed});

			if(type != active_type){
				throw std::logic_error("type of input is ["
					+ active_type.pretty_name()
					+ "] but connected output is of type ["
					+ type.pretty_name() + "]");
			}
		}

		auto const make_fn = [](auto t){
				return [](output_base* output_ptr)->result_type{
					return input_construct_data< Name, typename
						decltype(t)::type, IsRequired >{output_ptr};
				};
			};

		auto const type_to_data = hana::unpack(converter.types,
			[make_fn](auto ... t){
				return std::unordered_map<
						type_index, result_type(*)(output_base*)
					>{{
						type_index::type_id< typename decltype(t)::type >(),
						make_fn(t)
					} ...};
			});

		auto const active_type = converter.packed_index_to_type_index.at(
			packed_index{dims, converter.numbers.packed});

		return hana::append(std::move(previous_makers), hana::make_pair(dims,
			type_to_data.at(active_type)(output_ptr)));

	}


}


#endif
