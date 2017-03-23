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

#include <io_tools/make_string.hpp>

#include <optional>


namespace disposer{


	template < typename Name, typename ... T >
	class parameter{
	public:
		static_assert(hana::is_a< parameter_name_tag, Name >);


		using name_type = Name;

		static constexpr std::string_view name{ name_type::value.c_str() };


		static constexpr auto types = hana::tuple_t< T ... >;

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::parameter needs all types T to be unique");

		static_assert(type_count != 0,
			"disposer::parameter needs at least on type");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::parameter types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::parameter types must not be references");


		template < typename EnableFunction, typename ParserFunction >
		parameter(
			EnableFunction&& enable_fn,
			ParserFunction&& parser_fn,
			std::string const& value
		):
			type_value_map_(hana::make_map(hana::make_pair(
				hana::type_c< T >,
				enable_fn(hana::type_c< T >)
				? std::optional< T const >(parser_fn(value, hana::type_c< T >))
				: std::optional< T const >()) ...
			)){}


		decltype(auto) operator()()const{
			static_assert(type_count == 1);
			return (*this)(types[hana::int_c< 0 >]);
		}

		template < typename Type >
		decltype(auto) operator()(Type const& type)const{
			if(!type_value_map_[type]){
				throw std::logic_error(io_tools::make_string(
					"access parameter '", name, "' with disabled type [",
					type_name< typename Type::type >(), "]"
				));
			}
			return *type_value_map_[type];
		}


	private:
		decltype(hana::make_map(hana::make_pair(hana::type_c< T >,
			std::declval< std::optional< T const > >()) ... ))
				const type_value_map_;
	};


	struct param_tag{};


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename EnableFunction,
		typename ParserFunction,
		typename ParameterType >
	struct param_t{
		/// \brief Tag for boost::hana
		using hana_tag = param_tag;

		/// \brief Parameter name as compile time string
		using name = Name;

		/// \brief Type of a disposer::parameter
		using type = ParameterType;

		/// \brief Enable function
		EnableFunction enabler;

		/// \brief Parameter parser function
		ParserFunction parser;
	};


	template < typename EnableFunction, typename ParserFunction >
	struct verify_parameter{
		template < typename T >
		constexpr void operator()(hana::basic_type< T > type)const{
			constexpr auto is_enable_callable_with =
				hana::is_valid([](auto&& type)->decltype((void)
					std::declval< EnableFunction >()(type)){});

			constexpr auto is_parser_callable_with =
				hana::is_valid([](auto&& type)->decltype(
					(void)std::declval< ParserFunction >()
						(std::declval< std::string >(), type)){});

			static_assert(is_enable_callable_with(hana::type_c< T >),
				"need enable_fn signature 'bool enable_fn("
				"hana::basic_type< T >)'");

			static_assert(is_parser_callable_with(hana::type_c< T >),
				"need parser_fn signature 'T parser_fn(std::string, "
				"hana::basic_type< T >)'");

			using enable_return_type =
				decltype(std::declval< EnableFunction >()(type));

			using parser_return_type =
				decltype(std::declval< ParserFunction >()
					(std::declval< std::string >(), type));

			static_assert(std::is_same_v< bool, enable_return_type >,
				"need enable_fn signature 'bool enable_fn("
				"hana::basic_type< T >)' (return type must be bool)");

			static_assert(std::is_same_v< typename decltype(+type)::type,
				parser_return_type >,
				"need parser_fn signature 'T parser_fn(std::string, "
				"hana::basic_type< T >)' (wrong return type)");
		}
	};


	template < char ... C >
	template <
		typename Types,
		typename EnableFunction,
		typename ParserFunction >
	constexpr auto parameter_name< C ... >::operator()(
		Types const& types,
		EnableFunction&& enable_fn,
		ParserFunction&& parser_fn
	)const noexcept{
		using name_type = parameter_name< C ... >;

		constexpr verify_parameter< EnableFunction, ParserFunction > verify;

		if constexpr(hana::is_a< hana::type_tag, Types >){
			verify(types);

			using type_parameter =
				parameter< name_type, typename Types::type >;

			return param_t< name_type, EnableFunction, ParserFunction,
				type_parameter >{
					static_cast< EnableFunction&& >(enable_fn),
					static_cast< ParserFunction&& >(parser_fn)};
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));

			hana::for_each(types, verify);

			auto unpack_types = hana::concat(
				hana::tuple_t< name_type >,
				hana::to_tuple(types));

			auto type_parameter =
				hana::unpack(unpack_types, hana::template_< parameter >);

			return param_t< name_type, EnableFunction, ParserFunction,
				typename decltype(type_parameter)::type >{
					static_cast< EnableFunction&& >(enable_fn),
					static_cast< ParserFunction&& >(parser_fn)};
		}
	}


}


#endif
