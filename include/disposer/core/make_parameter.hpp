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


	template <
		typename T,
		typename DimensionList,
		std::size_t ... VDs,
		typename ParserFn,
		typename DefaultValueFn,
		typename Accessory >
	static T get_parameter_value(
		DimensionList const&,
		dimension_dependancy< VDs ... > const&,
		parser_fn< ParserFn > const& parser,
		default_value_fn< DefaultValueFn > const& default_value_generator,
		Accessory const& accessory,
		parameter_data const* param_data_ptr
	){
		constexpr auto type = [](auto i){
				return DimensionList::dimensions[i][hana::size_c< 0 >];
			};

		if(param_data_ptr != nullptr && param_data_ptr->generic_value){
			return parser(accessory,
				*param_data_ptr->generic_value, hana::type_c< T >,
				type(hana::size_c< VDs >) ...);
		}

		if constexpr(
			auto const is_void_r = default_value_generator.is_void_r(
				accessory, hana::type_c< T >, type(hana::size_c< VDs >) ...);
			is_void_r
		){
			throw std::runtime_error("config value required");
		}else{
			return default_value_generator(
				accessory, hana::type_c< T >, type(hana::size_c< VDs >) ...);
		}
	}


}


#endif
