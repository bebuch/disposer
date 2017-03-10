//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
/// \file disposer/module.hpp Include this file in your module file
#ifndef _disposer__module__hpp_INCLUDED_
#define _disposer__module__hpp_INCLUDED_

#include "input.hpp"
#include "output.hpp"
#include "module_base.hpp"


namespace disposer{


	template < typename Name, typename Outputs, typename Inputs >
	class module: public module_base{
	public:
		static_assert(hana::is_a< hana::string_tag, Name >);

		module(make_data const& data):
			module_base(data, generate_input_list(), generate_output_list()){}

		Inputs in;
		Outputs out;


	private:
		input_list generate_input_list(){
			return hana::unpack(in, [](auto const& ... in){
				return input_list{ std::cref(hana::second(in)) ... };
			});
		}

		output_list generate_output_list(){
			return hana::unpack(out, [](auto const& ... out){
				return output_list{ std::cref(hana::second(out)) ... };
			});
		}
	};


}


namespace disposer::interface::module{


	template < typename Name, typename ... IO >
	constexpr auto module(Name&&, io< IO > ... ios){
		static_assert(hana::is_a< hana::string_tag, Name >,
			"Name must be of type boost::hana::string< ... >");

		using raw_name = std::remove_cv_t< std::remove_reference_t< Name > >;
		auto constexpr io_lists = make_io_lists(ios ...);
		auto constexpr in = hana::first(io_lists);
		auto constexpr out = hana::second(io_lists);

		return hana::type_c< ::disposer::module<
				raw_name,
				typename decltype(+in)::type,
				typename decltype(+out)::type
			> >;
	}


}


#endif
