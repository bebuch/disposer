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

#include "output_info.hpp"
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
			std::size_t number,
			Inputs&& inputs,
			Outputs&& outputs,
			Parameters&& parameters
		): module_base(
			Name::value.c_str(), chain, number,
			generate_input_list(), generate_output_list()),
			in(std::move(inputs)),
			out(std::move(outputs)),
			params(std::move(parameters)){}


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


		constexpr auto get_input_names()const{
			auto input_names = hana::keys(in);
			return input_names;
		}

		constexpr auto get_output_names()const{
			auto output_names = hana::keys(out);
			return output_names;
		}

		constexpr auto get_parameter_names()const{
			auto parameter_names = hana::keys(params);
			return parameter_names;
		}


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


	template < typename Makers >
	struct module_maker{
		Makers makers;

		module_ptr operator()(make_data const& data)const{
			// create inputs, outputs and parameter in the order of there
			// definition in the module
			auto iop_list = hana::fold_left(makers, hana::make_tuple(),
				[&data](auto&& get, auto&& maker){
					constexpr auto is_input =
						hana::is_a< input_maker_tag >(maker);
					constexpr auto is_output =
						hana::is_a< output_maker_tag >(maker);
					constexpr auto is_parameter =
						hana::is_a< parameter_maker_tag >(maker);

					static_assert(is_input || is_output || is_parameter,
						"maker is not an iop (this is a bug in disposer!)");

					if constexpr(is_input){
						auto iter = data.inputs.find(maker.name.c_str());
						auto output = iter == data.inputs.end()
							? output_base*(nullptr) : iter->second;

						auto info = output
							? std::optional< output_info >(
								output->enabled_types())
							: std::optional< output_info >();

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(iop_list, output, info));
					}else if constexpr(is_output){
						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(iop_list));
					}else{
						auto iter = data.parameters.find(maker.name.c_str());
						auto value = iter == data.parameters.end()
							? std::optional< std::string >()
							: std::make_optional(iter->second);

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(iop_list, value));
					}
				}
			);

			// A helper function
			auto as_map = [](auto&& xs){
				return hana::to_map(hana::transform(
					static_cast< decltype(xs)&& >(xs),
					[](auto&& x){
						return hana::make_pair(x.name.value, std::move(x));
					}));
			};

			// Create the module
			auto module = std::make_unique< module >(
				data.chain, data.number,
				as_map(hana::filter(
					std::move(iop_list), hana::is_a< input_tag >)),
				as_map(hana::filter(
					std::move(iop_list), hana::is_a< output_tag >)),
				as_map(hana::filter(
					std::move(iop_list), hana::is_a< parameter_tag >))
			);

			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			// TODO: Do this checks as first action in this function, because
			//       we can report multiple errors/warnings with this and it is
			//       better to do it as soon as possible, so the user can fix
			//       more failures with one run
			std::set< std::string > input_name_list = data.inputs;
			hana::for_each(module->get_input_names(),
				[&input_name_list](auto const& name){
					input_name_list.erase(name.c_str());
				});
			for(auto const& in: input_name_list){
				logsys::log([&data, &in](logsys::stdlogb& os){
					os << data.location() << "ERROR: input '"
						<< in << "' doesn't exist";
				});
			}

			std::set< std::string > output_name_list;
			std::transform(data.outputs.begin(), data.outputs.end(),
				std::back_inserter(output_name_list),
				[](auto const& pair){ return pair.first; });
			hana::for_each(module->get_output_names(),
				[&output_name_list](auto const& name){
					output_name_list.erase(name.c_str());
				});
			for(auto const& out: output_name_list){
				logsys::log([&data, &out](logsys::stdlogb& os){
					os << data.location() << "ERROR: output '"
						<< out << "' doesn't exist";
				});
			}

			std::set< std::string > parameter_name_list;
			std::transform(data.parameters.begin(), data.parameters.end(),
				std::back_inserter(parameter_name_list),
				[](auto const& pair){ return pair.first; });
			hana::for_each(module->get_parameter_names(),
				[&parameter_name_list](auto const& name){
					parameter_name_list.erase(name.c_str());
				});
			for(auto const& param: parameter_name_list){
				logsys::log([&data, &param](logsys::stdlogb& os){
					os << data.location() << "Warning: parameter '"
						<< param << "' doesn't exist";
				});
			}

			if(!input_name_list.empty() || !output_name_list.empty()){
				throw std::logic_error(data.location() + "some inputs or "
					"outputs don't exist, see previos log messages for more "
					"details");
			}

			return module;
		}
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
