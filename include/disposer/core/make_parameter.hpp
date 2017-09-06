//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_parameter__hpp_INCLUDED_
#define _disposer__core__make_parameter__hpp_INCLUDED_

#include "parameter_name.hpp"
#include "dimension_referrer.hpp"

#include "../config/module_make_data.hpp"

#include <variant>


namespace disposer{


	using namespace std::literals::string_view_literals;


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename DimensionReferrer,
		typename DimensionDependancy,
		typename ParserFn,
		typename DefaultValueFn >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter parser function
		parser_fn< ParserFn > parser;

		/// \brief Default value function
		default_value_fn< DefaultValueFn > default_value_generator;
	};


	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		std::size_t ... VD,
		typename ParserFn,
		typename DefaultValueFn >
	constexpr auto create_parameter_maker(
		parameter_name< C ... >,
		dimension_referrer< Template, D ... > const&,
		dimension_dependancy< VD ... > const&,
		parser_fn< ParserFn >&& parser,
		default_value_fn< DefaultValueFn >&& default_value_generator
	){
		// Check that dimension_referrer and dimension_dependancy are
		// independent
		constexpr auto ds = hana::tuple_c< D ... >;
		auto const is_independent = hana::all_of(hana::tuple_c< VD ... >,
			[](auto vd){ return !hana::contains(ds, vd); });
		static_assert(is_independent,
			"dimension_dependancy must not contain any dimension which is "
			"already contained in the dimension_referrer");

		return parameter_maker<
				output_name< C ... >,
				dimension_referrer< Template, D ... >,
				dimension_dependancy< VD ... >,
				ParserFn,
				DefaultValueFn
			>{
				std::move(parser),
				std::move(default_value_generator)
			};
	}


	/// \brief Creates a \ref parameter_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		typename ... Args >
	constexpr auto make(
		parameter_name< C ... >,
		dimension_referrer< Template, D ... > const&,
		Args&& ... args
	){
		detail::validate_arguments< dimension_dependancy_tag, parser_fn_tag,
			default_value_fn_tag >(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_parameter_maker(
			parameter_name< C ... >{},
			types,
			get_or_default(std::move(arg_tuple),
				hana::is_a< dimension_dependancy_tag >,
				dimension_dependancy{}),
			get_or_default(std::move(arg_tuple),
				hana::is_a< parser_fn_tag >,
				stream_parser),
			get_or_default(std::move(arg_tuple),
				hana::is_a< default_value_fn_tag >,
				auto_default))
		);
	}


	inline parameter_data const* get_parameter_data(
		parameter_list const& parameters,
		std::string_view const& name
	){
		auto const iter = parameters.find(name);
		return iter != parameters.end() ? &iter->second : nullptr;
	}


	template < typename Name, typename Type >
	struct parameter_construct_data{
		static constexpr auto log_name = "parameter"sv;

		parameter_construct_data(Type&& value)noexcept
			: value(std::move(value)) {}

		Type value;
	};


	template < typename ... Ts >
	constexpr auto parameter_variant_type(hana::tuple< Ts ... >)noexcept{
		return hana::type_c< std::variant< parameter_construct_data<
			Name, typename Ts::type > ... > >;
	}

	template < typename Name, typename Types >
	using parameter_variant = typename
		decltype(parameter_variant_type< Name >(Types{}))::type;

	template <
		typename ResultType,
		typename Name,
		typename DimensionReferrer,
		std::size_t ... VTs,
		typename ParserFn,
		typename DefaultValueFn,
		typename T >
	ResultType parameter_make_fn(){
		hana::basic_type< ResultType >,
		parameter_maker<
			Name,
			DimensionReferrer,
			dimension_dependancy< VTs ... >,
			ParserFn,
			DefaultValueFn > const& maker,
		parameter_data const* param_data_ptr,
		hana::basic_type< T > type
	){
		T value = [&maker, param_data_ptr, type](){
				if(param_data_ptr != nullptr){
					// TODO: Parse specialized values

					if(param_data_ptr->generic_value){
						return maker.parser(
							accessory,
							*param_data_ptr->generic_value,
							type);
					}

					return hana::unpack(
						dimension_dependancy< VTs ... >::dimension_numbers,
						 [](auto ... vts){
							if constexpr(
								maker.default_value_generator.is_void_r(
									accessory, type,
									dimension_list< Ds ... >::dimensions[vts])
							){
								throw std::runtime_error("");
							}else{

							}
						});

				}

				if constexpr()
			}();

		return output_construct_data< Name, type >{param_data_ptr};
	}

	template <
		typename Name,
		typename DimensionReferrer,
		typename DimensionDependancy,
		typename ParserFn,
		typename DefaultValueFn,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto make_construct_data(
		parameter_maker<
			Name,
			DimensionReferrer,
			DimensionDependancy,
			ParserFn,
			DefaultValueFn > const& maker,
		dimension_list< Ds ... >,
		module_make_data const& data,
		partial_deduced_list_index< KDs ... > const& dims,
		hana::tuple< Ts ... >&& previous_makers
	){
		auto constexpr converter =
			DimensionReferrer::template convert< dimension_list< Ds ... > >;
		using result_type =
			parameter_variant< Name, decltype(converter.types) >;

		auto const active_type = converter.to_type_index(
			packed_index{dims, converter.packed});

		auto const param_data_ptr = get_parameter_data(data.parameters,
			detail::to_std_string_view(Name{}));

		auto const make_fn = [](auto t){
				return [](
					parameter_maker< Name, DimensionReferrer > const& maker,
					parameter_data const* param_data_ptr
				)->result_type{
					using type = typename decltype(t)::type;

					if(param_data_ptr != nullptr){
						if(param_data_ptr->generic_value){
							maker.parser(*param_data_ptr->generic_value);
						}
					}else{

					}


					return output_construct_data< Name, type >{param_data_ptr};
				};
			};

		auto const type_to_data = hana::unpack(converter.types,
			[make_fn](auto ... t){
				return std::unordered_map<
						type_index, result_type(*)(
							parameter_maker< Name, DimensionReferrer > const&,
							parameter_data const*)
					>{{
						type_index::type_id< typename decltype(t)::type >(),
						make_fn(t)
					} ...};
			});

		return hana::append(std::move(previous_makers), hana::make_pair(dims,
			type_to_data.at(active_type)(maker, param_data_ptr)));
	}



}


#endif
