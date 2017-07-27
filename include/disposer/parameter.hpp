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
#include "iop_accessory.hpp"
#include "merge.hpp"
#include "as_text.hpp"

#include <io_tools/make_string.hpp>

#include <optional>


namespace disposer{


	namespace impl{


		template < typename T >
		struct is_optional: std::false_type{};

		template < typename T >
		struct is_optional< std::optional< T > >: std::true_type{};

		template < typename T >
		constexpr bool is_optional_v = is_optional< T >::value;


		struct hana_is_optional_t{
			template < typename T >
			constexpr auto operator()(hana::basic_type< T >)const noexcept{
				if constexpr(is_optional_v< T >){
					return hana::true_c;
				}else{
					return hana::false_c;
				}
			}
		};

		constexpr auto hana_is_optional = hana_is_optional_t{};


		struct hana_remove_optional_t{
			template < typename T >
			constexpr auto operator()(hana::basic_type< T >)const noexcept{
				if constexpr(is_optional_v< T >){
					return hana::type_c< typename T::value_type >;
				}else{
					return hana::type_c< T >;
				}
			}
		};

		constexpr auto hana_remove_optional = hana_remove_optional_t{};


	}


	template < typename Name, typename TypeTransformFn, typename ... T >
	class parameter{
	public:
		static_assert(hana::is_a< parameter_name_tag, Name >);


		/// \brief Hana tag to identify parameters
		using hana_tag = parameter_tag;


		/// \brief Compile time name of the parameter
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		/// \brief Meta function to transfrom subtypes to the actual types
		static constexpr auto type_transform =
			type_transform_fn< TypeTransformFn >{};

		/// \brief Subtypes (before type_transform) as hana::tuple
		static constexpr auto subtypes = hana::tuple_t< T ... >;

		/// \brief Types (after type_transform) as hana::tuple
		static constexpr auto types =
			hana::transform(subtypes, type_transform);

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(hana::length(subtypes) ==
			hana::length(hana::to_set(subtypes)),
			"disposer::parameter needs all subtypes T to be distinct");

		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::parameter needs all types T to be distinct");

		static_assert(type_count != 0,
			"disposer::parameter needs at least on type");

		static_assert(!hana::any_of(subtypes, hana::traits::is_const),
			"disposer::parameter subtypes must not be const");

		static_assert(!hana::any_of(subtypes, hana::traits::is_reference),
			"disposer::parameter subtypes must not be references");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::parameter types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::parameter types must not be references");


		using type_value_map_t =
			decltype(hana::make_map(hana::make_pair(
				type_transform(hana::type_c< T >),
				std::declval< std::optional<
					typename decltype(hana::traits::add_const(
						type_transform(hana::type_c< T >)
					))::type > >()) ... ));


		template <
			typename Maker,
			typename IOP_Accessory,
			typename Type >
		static std::optional< Type const > make_value(
			Maker const& maker,
			IOP_Accessory const& iop_accessory,
			std::optional< std::string_view > value,
			std::string const& name,
			hana::basic_type< Type > type
		){
			if(!maker.enable(iop_accessory, type,
				to_std_string_view(maker.to_text[type]))) return {};
			if(value) return maker.parser(iop_accessory, *value, type);
			auto const have_default_fn =
				!maker.default_value_generator.is_void_r(iop_accessory, type);
			if constexpr(have_default_fn){
				auto value = maker.default_value_generator(iop_accessory, type);
				if(value) return *value;
			}
			throw std::logic_error("parameter(" + name + ") is required");
		}


		/// \brief Constructor
		template < typename MakeData >
		parameter(MakeData const& m):
			type_value_map_(hana::unpack(hana::transform(types, [&](auto type){
					auto value = make_value(
						m.data.maker, m.accessory, m.data.value_map[type],
						to_std_string(m.data.maker.name), type);
					if(value){
						m.data.maker.value_verify(m.accessory, *value);
					}
					return hana::make_pair(type, std::move(value));
				}), hana::make_map)) {}

		/// \brief Parameters are not copyable
		parameter(parameter const&) = delete;

		/// \brief Parameters are not movable
		parameter(parameter&&) = delete;


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
					"accessed parameter '", to_std_string_view(name),
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
		typename ParameterType,
		typename ValueVerifyFn,
		typename EnableFn,
		typename ParserFn,
		typename DefaultValueFn,
		typename TypeToText >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter name as compile time string
		using name_type = typename ParameterType::name_type;

		/// \brief Name as hana::string
		static constexpr auto name = name_type::value;

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

		/// \brief Default value function
		default_value_fn< DefaultValueFn > default_value_generator;

		/// \brief hana::map from hana::type to hana::string
		TypeToText to_text;
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
		value_verify_fn< ValueVerifyFn >&& value_verify,
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
					hana::contains(keys, impl::hana_remove_optional(type)),
					hana::contains(as_text, impl::hana_remove_optional(type))
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
						AsText{}[impl::hana_remove_optional(type)]);
				}else{
					return hana::make_pair(type_transform(type),
						as_text[impl::hana_remove_optional(type)]);
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
				std::move(value_verify),
				std::move(enable),
				std::move(parser),
				std::move(default_value_generator),
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
		typename Arg6,
		typename Arg7 >
	constexpr auto parameter_name< C ... >::operator()(
		Types const& types,
		Arg2&& arg2,
		Arg3&& arg3,
		Arg4&& arg4,
		Arg5&& arg5,
		Arg6&& arg6,
		Arg7&& arg7
	)const{
		constexpr auto valid_argument = [](auto const& arg){
				return hana::is_a< type_transform_fn_tag >(arg)
					|| hana::is_a< value_verify_fn_tag >(arg)
					|| hana::is_a< enable_fn_tag >(arg)
					|| hana::is_a< parser_fn_tag >(arg)
					|| hana::is_a< default_value_fn_tag >(arg)
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
		auto const arg7_valid = valid_argument(arg7);
		static_assert(arg7_valid, "argument 7 is invalid");

		auto args = hana::make_tuple(
			static_cast< Arg2&& >(arg2),
			static_cast< Arg3&& >(arg3),
			static_cast< Arg4&& >(arg4),
			static_cast< Arg5&& >(arg5),
			static_cast< Arg6&& >(arg6),
			static_cast< Arg7&& >(arg7)
		);

		auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(tt, "more than one type_transform_fn");
		auto vv = hana::count_if(args, hana::is_a< value_verify_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(vv, "more than one value_verify_fn");
		auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(ef, "more than one enable_fn");
		auto pf = hana::count_if(args, hana::is_a< parser_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(pf, "more than one parser_fn");
		auto ct = hana::count_if(args, hana::is_a< default_value_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(ct, "more than one default_value_fn");
		auto tm = hana::count_if(args, hana::is_a< type_as_text_map_tag >)
			<= hana::size_c< 1 >;
		static_assert(tm, "more than one type_as_text_map");

		return create_parameter_maker(
			(*this),
			types,
			get_or_default(std::move(args),
				hana::is_a< type_transform_fn_tag >,
				type_transform_fn< no_transform >{}),
			get_or_default(std::move(args),
				hana::is_a< value_verify_fn_tag >,
				value_verify_fn< value_verify_always >{}),
			get_or_default(std::move(args),
				hana::is_a< enable_fn_tag >,
				enable_always),
			get_or_default(std::move(args),
				hana::is_a< parser_fn_tag >,
				parser_fn< stream_parser >{}),
			get_or_default(std::move(args),
				hana::is_a< default_value_fn_tag >,
				no_default),
			get_or_default(std::move(args),
				hana::is_a< type_as_text_map_tag >,
				type_as_text())
		);
	}

	template < typename Makers >
	auto check_parameters(
		std::string const& location,
		Makers const& makers,
		parameter_list const& params
	){
		auto parameters_names = hana::transform(
			hana::filter(makers, hana::is_a< parameter_maker_tag >),
			[](auto const& parameters_maker){
				return parameters_maker.name;
			});

		std::set< std::string > parameter_name_list;
		std::transform(params.begin(), params.end(),
			std::inserter(parameter_name_list, parameter_name_list.end()),
			[](auto const& pair){ return pair.first; });
		hana::for_each(parameters_names,
			[&parameter_name_list](auto const& name){
				parameter_name_list.erase(to_std_string(name));
			});

		for(auto const& param: parameter_name_list){
			logsys::log([&location, &param](logsys::stdlogb& os){
				os << location << "parameter("
					<< param << ") doesn't exist (WARNING)";
			});
		}
	}


}


#endif
