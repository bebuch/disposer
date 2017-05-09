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


		/// \brief Parse enabled parameters and store them
		template <
			typename IOP_List,
			typename EnableFunction,
			typename ParserFunction,
			typename DefaultValues,
			typename Values >
		parameter(
			IOP_List const& iop_list,
			EnableFunction const& enable_fn,
			ParserFunction const& parser_fn,
			DefaultValues const& default_values,
			Values const& values
		):
			type_value_map_(hana::make_map(hana::make_pair(
				hana::type_c< T >,
				enable_fn(iop_list, hana::type_c< T >)
				? std::optional< T const >(
					values[hana::type_c< T >]
					? parser_fn(*values[hana::type_c< T >], hana::type_c< T >)
					: (default_values
						? (*default_values)[hana::type_c< T >]
						: std::optional< T const >()
					)
				)
				: std::optional< T const >(
					default_values
					? (*default_values)[hana::type_c< T >]
					: std::optional< T const >()
				)) ...
			)){}


		/// \brief true if any type is enabled, otherwise false
		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(type_value_map_));
		}

		/// \brief true if type is enabled, otherwise false
		template < typename Type >
		bool is_enabled(Type const& type)const{
			return static_cast< bool >(type_value_map_[type]);
		}


		/// \brief Access the value if parameter has only one type
		decltype(auto) operator()()const{
			static_assert(type_count == 1,
				"you must call with a type: parameter(hana::type_c< Type >)");
			return (*this)(types[hana::int_c< 0 >]);
		}

		/// \brief Access parameter of given type
		template < typename Type >
		decltype(auto) operator()(Type const& type)const{
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
		/// \brief Map parameter types to values
		decltype(hana::make_map(hana::make_pair(hana::type_c< T >,
			std::declval< std::optional< T const > >()) ... ))
				const type_value_map_;
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
		typename EnableFunction,
		typename ParserFunction,
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

		/// \brief Enable function
		EnableFunction enabler;

		/// \brief Parameter parser function
		ParserFunction parser;

		/// \brief Optional default values
		default_value_type< decltype(types) > default_values;

		/// \brief hana::map from hana::type to hana::string
		TypeToText to_text;

		template < typename IOP_List, typename Values >
		constexpr auto operator()(
			IOP_List const& iop_list,
			Values const& values
		)const{
			return type(iop_list, enabler, parser, default_values, values);
		}
	};


	template < char ... C >
	template <
		typename Types,
		typename EnableFunction,
		typename ParserFunction,
		typename DefaultValues,
		typename AsText >
	constexpr auto parameter_name< C ... >::operator()(
		Types const&,
		EnableFunction&& enable_fn,
		ParserFunction&& parser_fn,
		DefaultValues&& default_values,
		AsText&&
	)const noexcept{
		using name_type = parameter_name< C ... >;
		using enable_fn_t = std::remove_reference_t< EnableFunction >;
		using parser_fn_t = std::remove_reference_t< ParserFunction >;

		constexpr auto typelist = to_typelist(Types{});

		auto default_value_map = [typelist,
			default_values = static_cast< DefaultValues&& >(default_values)
		]()mutable{
			if constexpr(
				hana::type_c< DefaultValues > == hana::type_c< no_defaults >
			){
				(void)default_values; // silence clang ...
				(void)typelist; // silence clang ...
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
		}();

		static_assert(hana::is_a< hana::map_tag, AsText >,
			"AsText must be a hana::map of hana::type's and hana::string's");
		constexpr auto keys = hana::to_tuple(hana::keys(AsText{}));
		static_assert(
			hana::all_of(keys, hana::is_a< hana::type_tag >),
			"AsText must be a hana::map with hana::type's as keys");
		static_assert(hana::all_of(hana::values(AsText{}),
				hana::is_a< hana::string_tag >),
			"AsText must be a hana::map with hana::string's as values");
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
			hana::concat(hana::tuple_t< name_type >, typelist);

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
			parameter_maker< name_type,
				typename decltype(type_parameter)::type,
				enable_fn_t, parser_fn_t,
				std::remove_const_t< decltype(type_to_text) >
			>{
				static_cast< EnableFunction&& >(enable_fn),
				static_cast< ParserFunction&& >(parser_fn),
				std::move(default_value_map), type_to_text
			};
	}


}


#endif
