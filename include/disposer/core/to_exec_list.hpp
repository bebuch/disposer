//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__to_exec_list__hpp_INCLUDED_
#define _disposer__core__to_exec_list__hpp_INCLUDED_

#include "input.hpp"
#include "output.hpp"
#include "exec_input.hpp"
#include "exec_output.hpp"

#include <boost/hana/tuple.hpp>


namespace disposer{


	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename ModuleInitFn,
		typename ExecFn >
	class module;


	template < typename IOList >
	struct to_exec_list;

	template < typename ... Names, typename ... Ts, bool ... IsRequireds >
	struct to_exec_list< hana::tuple< input< Names, Ts, IsRequireds > ... > >{
		using type = hana::tuple< exec_input< Names, Ts, IsRequireds > ... >;
	};

	template < typename ... Names, typename ... Ts >
	struct to_exec_list< hana::tuple< output< Names, Ts > ... > >{
		using type = hana::tuple< exec_output< Names, Ts > ... >;
	};

	template <>
	struct to_exec_list< hana::tuple<> >{
		using type = hana::tuple<>;
	};

	template < typename IOList >
	using to_exec_list_t = typename to_exec_list< IOList >::type;


	template < typename IOList >
	struct to_exec_init_list;

	template < typename ... Names, typename ... Ts, bool ... IsRequireds >
	struct to_exec_init_list
		< hana::tuple< input< Names, Ts, IsRequireds > ... > >
	{
		using type = hana::tuple< hana::tuple<
				input< Names, Ts, IsRequireds > const&, output_map_type const&
			> ... >;
	};

	template < typename ... Names, typename ... Ts >
	struct to_exec_init_list
		< hana::tuple< output< Names, Ts > ... > >
	{
		using type = hana::tuple< hana::tuple<
				output< Names, Ts >&, output_map_type&
			> ... >;
	};

	template <>
	struct to_exec_init_list< hana::tuple<> >{
		using type = hana::tuple<>;
	};

	template < typename IOList >
	using to_exec_init_list_t = typename to_exec_init_list< IOList >::type;


}


#endif
