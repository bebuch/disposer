//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_maker__hpp_INCLUDED_
#define _disposer__core__output_maker__hpp_INCLUDED_

#include "output.hpp"

#include "../tool/validate_arguments.hpp"


namespace disposer{


	/// \brief Provid types for constructing an output
	template <
		typename OutputType,
		typename EnableFn >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;

		/// \brief Output name as compile time string
		using name_type = typename OutputType::name_type;

		/// \brief Name as hana::string
		static constexpr auto name = name_type::value;

		/// \brief Type of a disposer::output
		using type = OutputType;

		/// \brief Enable function
		enable_fn< EnableFn > enable;
	};


	/// \brief Helper function for \ref output_name::operator()
	template <
		typename Name,
		typename Types,
		typename TypeTransformFn,
		typename EnableFn >
	constexpr auto create_output_maker(
		Name const&,
		Types const&,
		type_transform_fn< TypeTransformFn >&&,
		enable_fn< EnableFn >&& enable
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name, TypeTransformFn >, typelist);

		constexpr auto type_output =
			hana::unpack(unpack_types, hana::template_< output >);

		return output_maker<
			typename decltype(type_output)::type, EnableFn >{
				std::move(enable)
			};
	}


	/// \brief Creates a \ref output_maker object
	template < char ... C, typename Types, typename ... Args >
	constexpr auto make(
		output_name< C ... >,
		Types const& types,
		Args&& ... args
	){
		detail::validate_arguments<
				type_transform_fn_tag,
				enable_fn_tag
			>(args ...);

		auto arg_tuple = hana::make_tuple(static_cast< Args&& >(args) ...);

		return create_output_maker(
			output_name< C ... >{},
			types,
			get_or_default(std::move(arg_tuple),
				hana::is_a< type_transform_fn_tag >,
				no_type_transform),
			get_or_default(std::move(arg_tuple),
				hana::is_a< enable_fn_tag >,
				enable_always)
		);
	}


}


#endif
