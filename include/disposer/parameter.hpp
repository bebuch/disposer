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

#include <io_tools/make_string.hpp>

#include <optional>


namespace disposer{


	struct parameter_tag{};

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
			typename DefaultValues >
		parameter(
			IOP_List const& iop_list,
			EnableFunction const& enable_fn,
			ParserFunction const& parser_fn,
			DefaultValues const& default_values,
			std::optional< std::string >&& value
		):
			type_value_map_(hana::make_map(hana::make_pair(
				hana::type_c< T >,
				enable_fn(iop_list, hana::type_c< T >)
				? std::optional< T const >(
					value
					? parser_fn(*value, hana::type_c< T >)
					: (default_values
						? std::get< T const >(*default_values)
						: std::optional< T const >()
					)
				)
				: std::optional< T const >(
					default_values
					? std::get< T const >(*default_values)
					: std::optional< T const >()
				)) ...
			)){}


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
					"access parameter '", name.c_str(),
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


	/// \brief Provid types for constructing an parameter
	template <
		typename Name,
		typename ParameterType,
		typename EnableFunction,
		typename ParserFunction >
	struct parameter_maker{
		/// \brief Tag for boost::hana
		using hana_tag = parameter_maker_tag;

		/// \brief Parameter name as compile time string
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;

		/// \brief Type of a disposer::parameter
		using type = ParameterType;

		/// \brief Type for default values
		using tuple_type = typename decltype(hana::unpack(
			hana::transform(type::types, hana::traits::add_const),
			hana::template_< std::tuple >))::type;

		/// \brief Enable function
		EnableFunction enabler;

		/// \brief Parameter parser function
		ParserFunction parser;

		/// \brief Optional default values
		std::optional< tuple_type > default_values;

		template < typename IOP_List >
		constexpr auto operator()(
			IOP_List const& iop_list,
			std::optional< std::string >&& value
		)const{
			return type(
				iop_list, enabler, parser, default_values, std::move(value));
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
		ParserFunction&& parser_fn,
		default_value_type< Types >&& default_values
	)const noexcept{
		using name_type = parameter_name< C ... >;

		if constexpr(hana::is_a< hana::type_tag, Types >){
			using type_parameter =
				parameter< name_type, typename Types::type >;

			return
				parameter_maker<
					name_type,
					type_parameter,
					std::remove_reference_t< EnableFunction >,
					std::remove_reference_t< ParserFunction >
				>{
					static_cast< EnableFunction&& >(enable_fn),
					static_cast< ParserFunction&& >(parser_fn),
					std::move(default_values)
				};
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));

			auto unpack_types = hana::concat(
				hana::tuple_t< name_type >,
				hana::to_tuple(types));

			auto type_parameter =
				hana::unpack(unpack_types, hana::template_< parameter >);

			return
				parameter_maker<
					name_type,
					typename decltype(type_parameter)::type,
					std::remove_reference_t< EnableFunction >,
					std::remove_reference_t< ParserFunction >
				 >{
					static_cast< EnableFunction&& >(enable_fn),
					static_cast< ParserFunction&& >(parser_fn),
					std::move(default_values)
				};
		}
	}


}


#endif
