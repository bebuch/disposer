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
#include "parser_fn.hpp"
#include "default_value_fn.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/get_or_default.hpp"
#include "../tool/validate_arguments.hpp"
#include "../tool/to_std_string_view.hpp"

#include <variant>
#include <unordered_map>


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
		auto const is_independent = hana::all_of(
			hana::tuple_c< std::size_t, VD ... >,
			[](auto vd){
				return !hana::contains(hana::tuple_c< std::size_t, D ... >, vd);
			});
		static_assert(is_independent,
			"dimension_dependancy must not contain any dimension which is "
			"already contained in the dimension_referrer");

		return parameter_maker<
				parameter_name< C ... >,
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
		dimension_referrer< Template, D ... > const& referrer,
		Args&& ... args
	){
		detail::validate_arguments< dimension_dependancy_tag, parser_fn_tag,
			default_value_fn_tag >(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_parameter_maker(
			parameter_name< C ... >{},
			referrer,
			get_or_default(std::move(arg_tuple),
				hana::is_a< dimension_dependancy_tag >,
				dimension_dependancy{}),
			get_or_default(std::move(arg_tuple),
				hana::is_a< parser_fn_tag >,
				stream_parser),
			get_or_default(std::move(arg_tuple),
				hana::is_a< default_value_fn_tag >,
				auto_default));
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


	template < typename Name, typename ... Ts >
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
		typename Accessory,
		typename ParserFn,
		typename DefaultValueFn,
		typename DimensionList,
		typename PartialDeducedListIndex,
		std::size_t ... VTs >
	struct make_parameter_fn{
		using fn_type = ResultType(*)(
			Accessory const&,
			parser_fn< ParserFn > const&,
			default_value_fn< DefaultValueFn > const&,
			parameter_data const*,
			PartialDeducedListIndex const&);

		template < typename T >
		static ResultType make(
			hana::basic_type< T > type,
			Accessory const& accessory,
			parser_fn< ParserFn > const& parser,
			default_value_fn< DefaultValueFn > const& default_value_generator,
			parameter_data const* param_data_ptr,
			PartialDeducedListIndex const& dims
		){
			return parameter_construct_data< Name, T >{
				value(type, accessory, parser,
					default_value_generator, param_data_ptr, dims)};
		}

		template < typename T >
		static T value(
			hana::basic_type< T >,
			Accessory const& accessory,
			parser_fn< ParserFn > const& parser,
			default_value_fn< DefaultValueFn > const& default_value_generator,
			parameter_data const* param_data_ptr,
			PartialDeducedListIndex const& dims
		){
			auto ti = [&dims](auto vt){
					return DimensionList::dimensions[vt].ti[dims.index[vt].i];
				};

			if(param_data_ptr != nullptr){
				// TODO: Parse specialized values

				if(param_data_ptr->generic_value){
					return parser(accessory,
						*param_data_ptr->generic_value, hana::type_c< T >,
						ti(hana::size_c< VTs >) ...);
				}
			}

			if constexpr(
				auto const is_void_r = default_value_generator.is_void_r(
					accessory, hana::type_c< T >, ti(hana::size_c< VTs >) ...);
				is_void_r
			){
				throw std::runtime_error("config value required");
			}else{
				return default_value_generator(
					accessory, hana::type_c< T >, ti(hana::size_c< VTs >) ...);
			}
		}

		template < typename T >
		static fn_type maker(hana::basic_type< T >){
			return [](
					Accessory const& accessory,
					parser_fn< ParserFn > const& parser,
					default_value_fn< DefaultValueFn > const& dvg,
					parameter_data const* pd,
					PartialDeducedListIndex const& dims
				){
					return make(hana::basic_type< T >{},
						accessory, parser, dvg, pd, dims);
				};
		}
	};


	template <
		typename Name,
		typename DimensionReferrer,
		std::size_t ... VTs,
		typename ParserFn,
		typename DefaultValueFn,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto make_construct_data(
		parameter_maker<
			Name,
			DimensionReferrer,
			dimension_dependancy< VTs ... >,
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

		using accessory_type = int; // TODO: make by previous_makers

		using parameter_value_t = make_parameter_fn<
			result_type, Name, accessory_type, ParserFn, DefaultValueFn,
			dimension_list< Ds ... >, partial_deduced_list_index< KDs ... >,
			VTs ... >;

		auto const type_to_data = hana::unpack(converter.types,
			[](auto ... t){
				return std::unordered_map<
						type_index, typename parameter_value_t::fn_type
					>{{
						type_index::type_id< typename decltype(t)::type >(),
						parameter_value_t::maker(t)
					} ...};
			});

		return hana::append(std::move(previous_makers), hana::make_pair(dims,
			type_to_data.at(active_type)(accessory_type{}, maker.parser,
				maker.default_value_generator, param_data_ptr, dims)));
	}



}


#endif
