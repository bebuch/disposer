//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__parameter__hpp_INCLUDED_
#define _disposer__parameter__hpp_INCLUDED_

#include "parameter_name.hpp"
#include "type_index.hpp"
#include "iop_list.hpp"
#include "merge.hpp"
#include "as_text.hpp"

#include <io_tools/make_string.hpp>

#include <optional>


namespace disposer{


	template < typename Name, typename ... T >
	class parameter{
	public:
		static_assert(hana::is_a< parameter_name_tag, Name >);


		using hana_tag = parameter_tag;


		/// \brief Compile time name of the parameter
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		/// \brief Types of the parameter
		static constexpr auto types = hana::tuple_t< T ... >;

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::parameter needs all types T to be distinct");

		static_assert(type_count != 0,
			"disposer::parameter needs at least on type");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::parameter types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::parameter types must not be references");


		using type_value_map_t =
			decltype(hana::make_map(hana::make_pair(hana::type_c< T >,
				std::declval< std::optional< T const > >()) ... ));


		/// \brief Constructor
		parameter(type_value_map_t&& type_value_map):
			type_value_map_(std::move(type_value_map)) {}


		/// \brief true if any type is enabled, otherwise false
		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(type_value_map_));
		}

		/// \brief true if type is enabled, otherwise false
		template < typename Type >
		bool is_enabled(Type const& type)const{
			auto const is_type_valid = hana::contains(type_value_map_, type);
			static_assert(is_type_valid, "type in not an input type");
			return static_cast< bool >(type_value_map_[type]);
		}


		/// \brief Access the value if parameter has only one type
		decltype(auto) get()const{
			static_assert(type_count == 1,
				"you must call with a type: get(hana::type_c< Type >)");
			return get(types[hana::int_c< 0 >]);
		}

		/// \brief Access parameter of given type
		template < typename Type >
		decltype(auto) get(Type const& type)const{
			if(!is_enabled(type)){
				throw std::logic_error(io_tools::make_string(
					"accessed parameter '", name.c_str(),
					"' with disabled type [",
					type_name< typename Type::type >(), "]"
				));
			}
			return *type_value_map_[type];
		}


	private:
		/// \brief Map of parameter types to values
		type_value_map_t const type_value_map_;
	};


	struct default_value_type_impl{
		template < typename ... T >
		constexpr auto operator()(T ... v)const noexcept{
			return hana::type_c< std::optional< decltype(hana::make_map(
				hana::make_pair(v, std::declval< typename T::type >()) ...
			)) > >;
		}
	};

	template < typename Types >
	using default_value_type = typename decltype(
		hana::unpack(Types{}, default_value_type_impl{}))::type;


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename ParameterType,
		typename ValueVerifyFn,
		typename EnableFn,
		typename ParserFn,
		typename TypeToText >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter name as compile time string
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;

		/// \brief Type of a disposer::parameter
		using type = ParameterType;

		/// \brief Possible types of the parameter value
		static constexpr auto types = type::types;

		/// \brief Function to verify the parameter value
		value_verify_fn< ValueVerifyFn > value_verify;

		/// \brief Enable function
		enable_fn< EnableFn > enable;

		/// \brief Parameter parser function
		parser_fn< ParserFn > parser;

		/// \brief Optional default values
		default_value_type< decltype(types) > default_values;

		/// \brief hana::map from hana::type to hana::string
		TypeToText to_text;

		template <
			typename IOP_List,
			typename Value,
			typename DefaultValues,
			typename T >
		std::optional< T const > make_value(
			IOP_List const& iop_list,
			Value const& value,
			DefaultValues const& default_values,
			hana::basic_type< T > type
		)const{
			if(!enable(iop_list, type)) return {};
			if(value) return parser(*value, type);
			if(default_values) return (*default_values)[type];
			return {};
		}

		template < typename IOP_List, typename Values >
		constexpr auto operator()(
			IOP_List const& iop_list,
			Values const& values
		)const{
			return type(hana::unpack(hana::transform(types, [&](auto type){
					auto value = make_value(
						iop_list,
						values[type],
						default_values,
						type);
					if(value) value_verify(iop_list, *value);
					return hana::make_pair(type, std::move(value));
				}), hana::make_map));
		}
	};


	template <
		typename Types,
		typename DefaultValues >
	constexpr auto make_default_value_map(
		Types const& /*types*/,
		DefaultValues&& default_values
	){
		constexpr auto typelist = to_typelist(Types{});

		if constexpr(
			hana::type_c< DefaultValues > == hana::type_c< no_defaults >
		){
			(void)default_values; // silence clang ...
			return default_value_type< decltype(typelist) >();
		}else{
			static_assert(hana::is_a< hana::tuple_tag, DefaultValues >,
				"DefaultValues must be a hana::tuple with a value for "
				"every of the parameters type's");
			static_assert(
				hana::length(DefaultValues{}) == hana::length(typelist),
				"DefaultValues must have the same number of values as the "
				"parameter has types");
			static_assert(
				hana::all_of(hana::zip(typelist, DefaultValues{}),
					[](auto&& tuple){
						using namespace hana::literals;
						return tuple[0_c] == hana::typeid_(tuple[1_c]);
					}
				),
				"DefaultValues must have the same types in the same order "
				"as parameter's type's");
			return std::make_optional(hana::unpack(
				static_cast< DefaultValues&& >(default_values),
				[](auto&& ... values){
					return hana::make_map(
						hana::make_pair(hana::typeid_(values),
							static_cast< decltype(values)&& >(values)) ...);
				}));
		}
	}


	template <
		typename Name,
		typename Types,
		typename ValueVerifyFn,
		typename EnableFn,
		typename ParserFn,
		typename DefaultValues,
		typename AsText >
	constexpr auto create_parameter_maker(
		Name const&,
		Types const& types,
		value_verify_fn< ValueVerifyFn >&& value_verify,
		enable_fn< EnableFn >&& enable,
		parser_fn< ParserFn >&& parser,
		default_values_tuple< DefaultValues >&& default_values,
		type_as_text_map< AsText >&&
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto keys = hana::to_tuple(hana::keys(AsText{}));
		static_assert(hana::is_subset(keys, typelist),
			"AsText must contain only types which are also in the parameter's "
			"type list");
		static_assert(hana::all_of(typelist, [keys](auto type){
				return hana::or_(
					hana::contains(keys, type), hana::contains(as_text, type)
				);
			}),
			"At least one of the parameter's types has neither a hana::string "
			"representation in the default disposer as_text-list nor in the "
			"parameters AsText-list");

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name >, typelist);

		constexpr auto type_parameter =
			hana::unpack(unpack_types, hana::template_< parameter >);


		constexpr auto type_to_text = hana::unpack(hana::transform(typelist,
			[](auto type){
				constexpr auto keys = hana::to_tuple(hana::keys(AsText{}));
				if constexpr(hana::contains(keys, type)){
					return hana::make_pair(type, AsText{}[type]);
				}else{
					return hana::make_pair(type, as_text[type]);
				}
			}), hana::make_map);

		constexpr auto text_list = hana::values(type_to_text);
		static_assert(hana::length(text_list) ==
			hana::length(hana::to_set(text_list)),
			"At least two of the parameter types have the same text "
			"representation, check the parameters AsText-list");

		return
			parameter_maker< Name,
				typename decltype(type_parameter)::type,
				ValueVerifyFn, EnableFn, ParserFn,
				std::remove_const_t< decltype(type_to_text) >
			>{
				std::move(value_verify),
				std::move(enable),
				std::move(parser),
				make_default_value_map(types, std::move(default_values.values)),
				type_to_text
			};
	}


	template < char ... C >
	template <
		typename Types,
		typename Arg2,
		typename Arg3,
		typename Arg4,
		typename Arg5,
		typename Arg6 >
	constexpr auto parameter_name< C ... >::operator()(
		Types const& types,
		Arg2&& arg2,
		Arg3&& arg3,
		Arg4&& arg4,
		Arg5&& arg5,
		Arg6&& arg6
	)const{
		constexpr auto valid_argument = [](auto const& arg){
				return hana::is_a< value_verify_fn_tag >(arg)
					|| hana::is_a< enable_fn_tag >(arg)
					|| hana::is_a< parser_fn_tag >(arg)
					|| hana::is_a< default_values_tuple_tag >(arg)
					|| hana::is_a< type_as_text_map_tag >(arg)
					|| hana::is_a< no_argument_tag >(arg);
			};

		auto const arg2_valid = valid_argument(arg2);
		static_assert(arg2_valid, "argument 2 is invalid");
		auto const arg3_valid = valid_argument(arg3);
		static_assert(arg3_valid, "argument 3 is invalid");
		auto const arg4_valid = valid_argument(arg4);
		static_assert(arg4_valid, "argument 4 is invalid");
		auto const arg5_valid = valid_argument(arg5);
		static_assert(arg5_valid, "argument 5 is invalid");
		auto const arg6_valid = valid_argument(arg6);
		static_assert(arg6_valid, "argument 6 is invalid");

		auto args = hana::make_tuple(
			static_cast< Arg2&& >(arg2),
			static_cast< Arg3&& >(arg3),
			static_cast< Arg4&& >(arg4),
			static_cast< Arg5&& >(arg5),
			static_cast< Arg6&& >(arg6)
		);

		auto vv = hana::count_if(args, hana::is_a< value_verify_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(vv, "more than one value_verify_fn");
		auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(ef, "more than one enable_fn");
		auto pf = hana::count_if(args, hana::is_a< parser_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(pf, "more than one parser_fn");
		auto ct = hana::count_if(args, hana::is_a< default_values_tuple_tag >)
			<= hana::size_c< 1 >;
		static_assert(ct, "more than one default_values_tuple");
		auto tt = hana::count_if(args, hana::is_a< type_as_text_map_tag >)
			<= hana::size_c< 1 >;
		static_assert(tt, "more than one type_as_text_map");

		return create_parameter_maker(
			(*this),
			types,
			get_or_default(std::move(args),
				hana::is_a< value_verify_fn_tag >,
				value_verify_fn< value_verify_always >{}),
			get_or_default(std::move(args),
				hana::is_a< enable_fn_tag >,
				enable_fn< enable_always >{}),
			get_or_default(std::move(args),
				hana::is_a< parser_fn_tag >,
				parser_fn< stream_parser >{}),
			get_or_default(std::move(args),
				hana::is_a< default_values_tuple_tag >,
				default_values_tuple< no_defaults >{}),
			get_or_default(std::move(args),
				hana::is_a< type_as_text_map_tag >,
				type_as_text())
		);
	}



}


#endif
