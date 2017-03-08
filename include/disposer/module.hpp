//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
/// \file disposer/module.hpp Include this file in your module file
#ifndef _disposer__io__hpp_INCLUDED_
#define _disposer__io__hpp_INCLUDED_

#include "input.hpp"
#include "output.hpp"
#include "module_base.hpp"
#include "disposer.hpp"


namespace disposer::interface::module{


	template < typename Derived >
	class module: public module_base{
	public:
		module(make_data const& data):
			module_base(data, {}, {}){}

	private:
		Derived& derived(){
			static_assert(std::is_base_of_v< module, Derived >);
			return *static_cast< Derived* >(this);
		}

		output_list generate_output_list(){
			return hana::unpack(derived().out, [](auto const& ... out){
				return output_list{ std::cref(hana::second(out)) ... };
			});
		}

		input_list generate_input_list(){
			return hana::unpack(derived().in, [](auto const& ... in){
				return input_list{ std::cref(hana::second(in)) ... };
			});
		}
	};


}


#endif
