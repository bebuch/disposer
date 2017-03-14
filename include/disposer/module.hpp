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
#include "module_name.hpp"


namespace disposer{


	template < typename Name, typename Inputs, typename Outputs >
	class module: public module_base{
	public:
		static_assert(hana::is_a< module_name_tag, Name >);

		using name = Name;


		module(make_data const& data):
			module_base(data, generate_input_list(), generate_output_list()){}

		Inputs in;
		Outputs out;


// 		template < typename InputName >
// 		bool is_input_active(InputName&& name){
// 			static_assert(hana::is_a< hana::string_tag >(name),
// 				"name must be of type boost::hana::string< ... >");
// 			static_assert(hana::contains(in, name), "name is not an input");
// 			return inputs_.find(name.c_str()) != inputs_.end();
// 		}
//
// 		template < typename OutputName >
// 		bool is_output_active(OutputName&& name){
// 			static_assert(hana::is_a< hana::string_tag >(name),
// 				"name must be of type boost::hana::string< ... >");
// 			static_assert(hana::contains(out, name), "name is not an output");
// 			return outputs_.find(name.c_str()) != outputs_.end();
// 		}


	private:
		input_list generate_input_list(){
			return hana::unpack(in, [](auto& ... in){
				return input_list{ hana::second(in) ... };
			});
		}

		output_list generate_output_list(){
			return hana::unpack(out, [](auto& ... out){
				return output_list{ hana::second(out) ... };
			});
		}
	};


	template < char ... C >
	template < typename ... IO >
	constexpr auto
	module_name< C ... >::operator()(io< IO > const& ... ios)const noexcept{
		using name_type = module_name< C ... >;
		auto io_lists = make_io_lists(ios ...);
		auto in = hana::first(io_lists);
		auto out = hana::second(io_lists);

		return hana::type_c< module<
				name_type,
				typename decltype(+in)::type,
				typename decltype(+out)::type
			> >;
	}


}


namespace disposer::interface::module{


	template < typename Name, typename Inputs, typename Outputs >
	constexpr auto make_register_fn(
		hana::basic_type< ::disposer::module< Name, Inputs, Outputs > >
	){
		return [](::disposer::module_declarant& add){
			add(Name::c_str(), [](::disposer::make_data const& data){
				return std::make_unique<
					::disposer::module< Name, Inputs, Outputs > >(data);
			});
		};
	}


}


#endif
