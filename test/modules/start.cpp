//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/disposer_module
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/module.hpp>

#include <boost/dll.hpp>


namespace disposer_module::start{


	using namespace disposer;
	using namespace disposer::literals;

	void init(std::string const& name, declarant& disposer){
		auto init = generate_module(
			"start module",
			module_configure(
				make("char"_param, free_type_c< char >, "a character"),
				make("char"_out, free_type_c< char >, "a character")
			),
			exec_fn([](auto module){
				module("char"_out).push(module("char"_param));
			})
		);

		init(name, disposer);
	}

	BOOST_DLL_AUTO_ALIAS(init)


}
