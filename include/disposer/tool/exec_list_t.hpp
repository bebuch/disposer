//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__exec_list_t__hpp_INCLUDED_
#define _disposer__tool__exec_list_t__hpp_INCLUDED_

#include "false_c.hpp"

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/unpack.hpp>


namespace disposer{


	namespace hana = boost::hana;


	struct input_tag;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class input;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class input_exec;


	struct output_tag;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class output;

	template < typename Name, typename TypeTransformFn, typename ... T >
	class output_exec;


}


namespace disposer::detail{


	constexpr auto add_types = [](auto const& list){
			return hana::transform(list, hana::make_type);
		};

	template < typename IO >
	struct to_exec{
		static_assert(false_c< IO >, "IO must be an input or an output");
	};

	template < typename Name, typename TypeTransformFn, typename ... T >
	struct to_exec< input< Name, TypeTransformFn, T ... > >{
		using type = input_exec< Name, TypeTransformFn, T ... >;
	};

	template < typename Name, typename TypeTransformFn, typename ... T >
	struct to_exec< output< Name, TypeTransformFn, T ... > >{
		using type = output_exec< Name, TypeTransformFn, T ... >;
	};

	template < typename IO >
	using to_exec_t = typename to_exec< IO >::type;


	constexpr auto is_input_or_output = [](auto const& t){
			return hana::is_a< input_tag,
					typename decltype(hana::typeid_(t))::type >
				|| hana::is_a< output_tag,
					typename decltype(hana::typeid_(t))::type >;
		};

	constexpr auto type_to_exec_type = [](auto ... type){
			return hana::type_c< hana::tuple<
				to_exec_t< typename decltype(type)::type > ... > >;
		};

	constexpr auto input_output_type_filter = [](auto const& list){
			return hana::filter(add_types(list), is_input_or_output);
		};


	template < typename List >
	using exec_list_t = typename decltype(hana::unpack(
		input_output_type_filter(std::declval< List >()),
		type_to_exec_type))::type;

	template < typename List >
	constexpr std::size_t input_output_count_c =
		decltype(hana::size(input_output_type_filter(
			std::declval< List >())))::value;


}


#endif
