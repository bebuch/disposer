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
			inputs(std::move(inputs)),
			outputs(std::move(outputs)),
			parameters(std::move(parameters)){}


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
				return inputs[io_t::value];
			}else if constexpr(hana::is_a< output_name_tag, io_t >){
				static_assert(have_output(io_t::value));
				return outputs[io_t::value];
			}else{
				static_assert(have_parameter(io_t::value));
				return parameters[io_t::value];
			}
		}


	private:
		input_list generate_input_list(){
			return hana::unpack(inputs, [](auto& ... input){
				return input_list{ hana::second(input) ... };
			});
		}

		output_list generate_output_list(){
			return hana::unpack(outputs, [](auto& ... output){
				return output_list{ hana::second(output) ... };
			});
		}


		template < typename String >
		constexpr static bool have_input(String){
			return hana::contains(decltype(hana::keys(inputs))(), String());
		}

		template < typename String >
		constexpr static bool have_output(String){
			return hana::contains(decltype(hana::keys(outputs))(), String());
		}

		template < typename String >
		constexpr static bool have_parameter(String){
			return hana::contains(decltype(hana::keys(parameters))(), String());
		}

		Inputs inputs;
		Outputs outputs;
		Parameters parameters;
	};


	template <
		typename Name,
		typename Makers >
	struct module_maker{
		/// \brief Output name as compile time string
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;

		/// \brief Tuple of input/output/parameter-maker objects
		Makers makers;

		auto operator()(make_data const& data)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto input_names = hana::transform(
				hana::filter(makers, hana::is_a< input_maker_tag >),
				[](auto const& input_maker){
					return input_maker.name;
				});

			std::set< std::string > input_name_list;
			std::transform(data.inputs.begin(), data.inputs.end(),
				std::inserter(input_name_list, input_name_list.end()),
				[](auto const& pair){ return pair.first; });
			hana::for_each(input_names,
				[&input_name_list](auto const& name){
					input_name_list.erase(name.c_str());
				});

			for(auto const& in: input_name_list){
				logsys::log([&data, &in](logsys::stdlogb& os){
					os << data.location() << "ERROR: input '"
						<< in << "' doesn't exist";
				});
			}


			auto output_names = hana::transform(
				hana::filter(makers, hana::is_a< output_maker_tag >),
				[](auto const& output_maker){
					return output_maker.name;
				});

			std::set< std::string > output_name_list = data.outputs;
			hana::for_each(output_names,
				[&output_name_list](auto const& name){
					output_name_list.erase(name.c_str());
				});

			for(auto const& out: output_name_list){
				logsys::log([&data, &out](logsys::stdlogb& os){
					os << data.location() << "ERROR: output '"
						<< out << "' doesn't exist";
				});
			}


			auto parameters_names = hana::transform(
				hana::filter(makers, hana::is_a< parameter_maker_tag >),
				[](auto const& parameters_maker){
					return parameters_maker.name;
				});

			std::set< std::string > parameter_name_list;
			std::transform(data.parameters.begin(), data.parameters.end(),
				std::inserter(parameter_name_list, parameter_name_list.end()),
				[](auto const& pair){ return pair.first; });
			hana::for_each(parameters_names,
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


			// create inputs, outputs and parameter in the order of there
			// definition in the module
			auto iop_list = hana::fold_left(makers, hana::make_tuple(),
				[&data](auto&& get, auto&& maker){
					auto is_input =
						hana::is_a< input_maker_tag >(maker);
					auto is_output =
						hana::is_a< output_maker_tag >(maker);
					auto is_parameter =
						hana::is_a< parameter_maker_tag >(maker);

					static_assert(is_input || is_output || is_parameter,
						"maker is not an iop (this is a bug in disposer!)");

					if constexpr(is_input){
						auto iter = data.inputs.find(maker.name.c_str());
						auto output = iter == data.inputs.end()
							? static_cast< output_base* >(nullptr)
							: iter->second;

						auto info = output
							? std::optional< output_info >(
								output->enabled_types())
							: std::optional< output_info >();

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(get), output, info));
					}else if constexpr(is_output){
						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(get)));
					}else{
						auto iter = data.parameters.find(maker.name.c_str());
						auto value = iter == data.parameters.end()
							? std::optional< std::string >()
							: std::make_optional(iter->second);

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(get), std::move(value)));
					}
				}
			);

			// A helper function
			auto as_map = [](auto&& xs){
				return hana::to_map(hana::transform(
					static_cast< decltype(xs)&& >(xs),
					[](auto&& x){
						return hana::make_pair(x.name, std::move(x));
					}));
			};

			auto inputs = as_map(hana::filter(
				std::move(iop_list), hana::is_a< input_tag >));
			auto outputs = as_map(hana::filter(
				std::move(iop_list), hana::is_a< output_tag >));
			auto parameters = as_map(hana::filter(
				std::move(iop_list), hana::is_a< parameter_tag >));

			// Create the module
			auto module_ptr = std::make_unique< module<
					name_type,
					decltype(inputs),
					decltype(outputs),
					decltype(parameters)
				> >(
					data.chain, data.number,
					std::move(inputs),
					std::move(outputs),
					std::move(parameters)
				);

			return module_ptr;
		}
	};


	template < char ... C >
	template < typename ... ConfigList >
	constexpr auto
	module_name< C ... >::operator()(ConfigList&& ... list)const noexcept{
		return module_maker<
				module_name< C ... >,
				hana::tuple< std::remove_reference_t< ConfigList > ... >
			>{hana::make_tuple(static_cast< ConfigList&& >(list) ...)};
	}


	template < typename Name, typename Maker >
	constexpr auto make_register_fn(
		module_maker< Name, Maker >&& maker
	)noexcept{
		return [maker = std::move(maker)](module_declarant& add){
			add(Name::value.c_str(), [&maker](make_data const& data){
					return maker(data);
				});
		};
	}


}


#endif
