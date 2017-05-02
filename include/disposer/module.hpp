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
#include "parameter.hpp"
#include "module_base.hpp"
#include "module_name.hpp"


namespace disposer{


	template <
		typename Name,
		typename Inputs,
		typename Outputs,
		typename Parameters >
	class module: public module_base{
	public:
		static_assert(hana::is_a< module_name_tag, Name >);

		using name_type = Name;


		module(
			std::string const& chain,
			std::size_t number
		): module_base(
			Name::value.c_str(), chain, number,
			generate_input_list(), generate_output_list()){}


		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&&){
			using io_t =
				std::remove_cv_t< std::remove_reference_t< ConfigList > >;
			static_assert(
				hana::is_a< input_name_tag, io_t > ||
				hana::is_a< output_name_tag, io_t > ||
				hana::is_a< parameter_name_tag, io_t >,
				"parameter io must be an input_name, an output_name or a"
				"parameter_name");

			if constexpr(hana::is_a< input_name_tag, io_t >){
				static_assert(have_input(io_t::value));
				return in[io_t::value];
			}else if constexpr(hana::is_a< output_name_tag, io_t >){
				static_assert(have_output(io_t::value));
				return out[io_t::value];
			}else{
				static_assert(have_parameter(io_t::value));
				return param[io_t::value];
			}
		}


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


		template < typename String >
		constexpr static bool have_input(String){
			return hana::contains(decltype(hana::keys(in))(), String());
		}

		template < typename String >
		constexpr static bool have_output(String){
			return hana::contains(decltype(hana::keys(out))(), String());
		}

		template < typename String >
		constexpr static bool have_parameter(String){
			return hana::contains(decltype(hana::keys(param))(), String());
		}

		Inputs in;
		Outputs out;
		Parameters param;
	};


	template < char ... C >
	template < typename ... ConfigList >
	constexpr auto
	module_name< C ... >::operator()(ConfigList&& ... list)const noexcept{
		using name_type = module_name< C ... >;
		auto config_lists =
			separate_module_config_lists(static_cast< ConfigList&& >(list) ...);

		using namespace hana::literals;
		auto in = config_lists[0_c];
		auto out = config_lists[1_c];
		auto param = config_lists[2_c];

		return hana::type_c< module<
				name_type,
				typename decltype(+in)::type,
				typename decltype(+out)::type,
				typename decltype(+param)::type
			> >;
	}


}


namespace disposer::interface::module{


	template <
		typename Name,
		typename Inputs,
		typename Outputs,
		typename Parameters >
	constexpr auto make_register_fn(
		hana::basic_type<
			::disposer::module< Name, Inputs, Outputs, Parameters > >
	){
		return [](::disposer::module_declarant& add){
			add(Name::value.c_str(), [](::disposer::make_data const& data){
				return std::make_unique<
					::disposer::module< Name, Inputs, Outputs, Parameters > >(
						data.chain, data.number
					);
			});
		};
	}


}


#endif
