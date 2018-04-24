//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
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
#include "dimension_referrer_output.hpp"
#include "parser_fn.hpp"
#include "default_value_fn.hpp"
#include "verify_value_fn.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/get_or_default.hpp"
#include "../tool/validate_arguments.hpp"
#include "../tool/to_std_string_view.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <variant>
#include <unordered_map>


namespace disposer{


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename DimensionReferrer,
		typename ParserFn,
		typename DefaultValueFn,
		typename DefaultValueHelpFn,
		typename VerfiyValueFn >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Dimension dependence of the type
		using dimension_referrer = DimensionReferrer;

		/// \brief parameter_name object
		static constexpr auto name = Name{};

		/// \brief True if is type is not dependet on anything
		static constexpr bool is_free_type = DimensionReferrer::is_free_type;


		/// \brief Description of the parameter
		template < typename ... DTs >
		std::string help_text_fn(dimension_list< DTs ... >)const{
			std::ostringstream help;
			help << "    * parameter: "
				<< detail::to_std_string_view(name) << "\n";
			help << help_text << "\n";
			help << wrapped_type_ref_text(
				DimensionReferrer{}, dimension_list< DTs ... >{});
			auto const default_value_help =
				default_value_generator.help_text_fn(
					DimensionReferrer{}, dimension_list< DTs ... >{});
			help << "      * " <<
				boost::replace_all_copy(default_value_help, "\n", "\n        ")
				<< "\n";
			return help.str();
		}

		/// \brief Parameter parser function
		parser_fn< ParserFn > parser;

		/// \brief Default value function
		default_value_fn< DefaultValueFn, DefaultValueHelpFn >
			default_value_generator;

		/// \brief Verfiy value function
		verify_value_fn< VerfiyValueFn > verify_value;

		/// \brief User defined help text
		std::string const help_text;
	};


	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		typename ParserFn,
		typename DefaultValueFn,
		typename DefaultValueHelpFn,
		typename VerfiyValueFn >
	auto create_parameter_maker(
		parameter_name< C ... >,
		dimension_referrer< Template, D ... > const&,
		std::string const& description,
		parser_fn< ParserFn > parser,
		default_value_fn< DefaultValueFn, DefaultValueHelpFn >
			default_value_generator,
		verify_value_fn< VerfiyValueFn > verify_value
	){
		return parameter_maker<
				parameter_name< C ... >,
				dimension_referrer< Template, D ... >,
				ParserFn,
				DefaultValueFn,
				DefaultValueHelpFn,
				VerfiyValueFn
			>{
				std::move(parser),
				std::move(default_value_generator),
				std::move(verify_value),
				"      * " +
				boost::replace_all_copy(description, "\n", "\n        ")
			};
	}


	/// \brief Creates a \ref parameter_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		typename ... Args >
	auto make(
		parameter_name< C ... >,
		dimension_referrer< Template, D ... > const& referrer,
		std::string const& description,
		Args&& ... args
	){
		detail::validate_arguments< parser_fn_tag,
			default_value_fn_tag, verify_value_fn_tag >(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_parameter_maker(
			parameter_name< C ... >{},
			referrer,
			description,
			get_or_default(std::move(arg_tuple),
				hana::is_a< parser_fn_tag >,
				stream_parser),
			get_or_default(std::move(arg_tuple),
				hana::is_a< default_value_fn_tag >,
				auto_default),
			get_or_default(std::move(arg_tuple),
				hana::is_a< verify_value_fn_tag >,
				verify_value_always));
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
		typename ParserFn,
		typename DefaultValueFn,
		typename DefaultValueHelpFn,
		typename VerfiyValueFn,
		typename Accessory >
	static T get_parameter_value(
		std::string_view parameter_name,
		DimensionList const&,
		parser_fn< ParserFn > const& parser,
		default_value_fn< DefaultValueFn, DefaultValueHelpFn > const&
			default_value_generator,
		verify_value_fn< VerfiyValueFn > const& verify_value,
		Accessory const& accessory,
		parameter_data const* param_data_ptr
	){
		if(param_data_ptr != nullptr && param_data_ptr->generic_value){
			T result(parser(parameter_name,
				*param_data_ptr->generic_value, hana::type_c< T >, accessory));

			verify_value(parameter_name, result, accessory);

			return result;
		}

		if constexpr(
			auto const is_void_r = default_value_generator
				.is_void_r(hana::type_c< T >, accessory);
			is_void_r
		){
			throw std::runtime_error(io_tools::make_string(
				"parameter(", parameter_name, ") config value required [",
				ct_pretty_name< T >(), "]"));
		}else{
			return default_value_generator(
				parameter_name, hana::type_c< T >, accessory);
		}
	}


}


#endif
