//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__parameter_maker__hpp_INCLUDED_
#define _disposer__core__parameter_maker__hpp_INCLUDED_

#include "parameter.hpp"
#include "as_text.hpp"

#include "../tool/remove_optional.hpp"
#include "../tool/validate_arguments.hpp"


namespace disposer{


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename DimensionConverter,
		bool IsRequired >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter parser function
		parser_fn< ParserFn > parser;

		/// \brief Default value function
		default_value_fn< DefaultValueFn > default_value_generator;
	};


	template <
		typename Name,
		typename Types,
		typename TypeTransformFn,
		typename ValueVerifyFn,
		typename EnableFn,
		typename ParserFn,
		typename DefaultValueFn,
		typename AsText >
	constexpr auto create_parameter_maker(
		Name const&,
		Types const&,
		type_transform_fn< TypeTransformFn >&&,
		verify_value_fn< ValueVerifyFn >&& verify_value,
		enable_fn< EnableFn >&& enable,
		parser_fn< ParserFn >&& parser,
		default_value_fn< DefaultValueFn >&& default_value_generator,
		type_as_text_map< AsText >&&
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto keys = hana::to_tuple(hana::keys(AsText{}));
		static_assert(hana::is_subset(keys, typelist),
			"AsText must contain only types which are also in the parameter's "
			"type list");
		static_assert(hana::all_of(typelist, [keys](auto type){
				return hana::or_(
					hana::contains(keys, detail::remove_optional(type)),
					hana::contains(as_text, detail::remove_optional(type))
				);
			}),
			"At least one of the parameter's types has neither a hana::string "
			"representation in the default disposer as_text-list nor in the "
			"parameters AsText-list");

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name, TypeTransformFn >, typelist);

		constexpr auto type_parameter =
			hana::unpack(unpack_types, hana::template_< parameter >);

		constexpr auto type_to_text = hana::unpack(hana::transform(typelist,
			[](auto type){
				constexpr auto type_transform =
					type_transform_fn< TypeTransformFn >{};
				constexpr auto keys = hana::to_tuple(hana::keys(AsText{}));
				if constexpr(hana::contains(keys, type)){
					return hana::make_pair(type_transform(type),
						AsText{}[detail::remove_optional(type)]);
				}else{
					return hana::make_pair(type_transform(type),
						as_text[detail::remove_optional(type)]);
				}
			}), hana::make_map);

		constexpr auto text_list = hana::values(type_to_text);
		static_assert(hana::length(text_list) ==
			hana::length(hana::to_set(text_list)),
			"At least two of the parameter types have the same text "
			"representation, check the parameters AsText-list");

		return parameter_maker<
				typename decltype(type_parameter)::type,
				ValueVerifyFn, EnableFn, ParserFn, DefaultValueFn,
				std::remove_const_t< decltype(type_to_text) >
			>{
				std::move(verify_value),
				std::move(enable),
				std::move(parser),
				std::move(default_value_generator),
				type_to_text
			};
	}


	/// \brief Creates a \ref parameter_maker object
	template < char ... C, typename Types, typename ... Args >
	constexpr auto make(
		parameter_name< C ... >,
		Types const& types,
		Args&& ... args
	){
		detail::validate_arguments<
				type_transform_fn_tag,
				verify_value_fn_tag,
				enable_fn_tag,
				parser_fn_tag,
				default_value_fn_tag,
				type_as_text_map_tag
			>(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_parameter_maker(
			parameter_name< C ... >{},
			types,
			get_or_default(std::move(arg_tuple),
				hana::is_a< type_transform_fn_tag >,
				no_type_transform),
			get_or_default(std::move(arg_tuple),
				hana::is_a< verify_value_fn_tag >,
				verify_value_always),
			get_or_default(std::move(arg_tuple),
				hana::is_a< enable_fn_tag >,
				enable_always),
			get_or_default(std::move(arg_tuple),
				hana::is_a< parser_fn_tag >,
				stream_parser),
			get_or_default(std::move(arg_tuple),
				hana::is_a< default_value_fn_tag >,
				auto_default),
			get_or_default(std::move(arg_tuple),
				hana::is_a< type_as_text_map_tag >,
				type_as_text())
		);
	}


}


#endif
