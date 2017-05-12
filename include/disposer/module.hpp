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

#include <type_traits>


namespace disposer{


	template <
		typename Inputs,
		typename Outputs,
		typename Parameters,
		typename EnableFunction >
	class module: public module_base{
	public:
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		static_assert(std::is_callable_v< EnableFunction(module const&) >);
#else
		static_assert(std::is_invocable_v< EnableFunction, module const& >);
#endif



		/// \brief Constructor
		module(
			std::string const& module_type,
			std::string const& chain,
			std::size_t number,
			Inputs&& inputs,
			Outputs&& outputs,
			Parameters&& parameters,
			EnableFunction const& enable_fn
		): module_base(
			module_type, chain, number,
			generate_input_list(), generate_output_list()),
			inputs_(std::move(inputs)),
			outputs_(std::move(outputs)),
			parameters_(std::move(parameters)),
			enable_fn_(enable_fn){}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&&)noexcept{
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
				return inputs_[io_t::value];
			}else if constexpr(hana::is_a< output_name_tag, io_t >){
				static_assert(have_output(io_t::value));
				return outputs_[io_t::value];
			}else{
				static_assert(have_parameter(io_t::value));
				return parameters_[io_t::value];
			}
		}


	private:
		/// \brief Enables the module for exec calls
		virtual void enable()override{
			exec_fn_.emplace(enable_fn_(static_cast< module const& >(*this)));
		}

		/// \brief Disables the module for exec calls
		virtual void disable()noexcept override{
			exec_fn_.reset();
		}


		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			if(exec_fn_){
				(*exec_fn_)(*this, id);
			}else{
				throw std::logic_error("module is not enabled");
			}
		}


		/// \brief std::vector with references to all input's (input_base)
		input_list generate_input_list(){
			return hana::unpack(inputs_, [](auto& ... input){
				return input_list{ hana::second(input) ... };
			});
		}

		/// \brief std::vector with references to all output's (output_base)
		output_list generate_output_list(){
			return hana::unpack(outputs_, [](auto& ... output){
				return output_list{ hana::second(output) ... };
			});
		}


		/// \brief hana::true_ if input_name exists
		template < typename String >
		constexpr static auto have_input(String){
			return
				hana::contains(decltype(hana::keys(inputs_))(), String());
		}

		/// \brief hana::true_ if output_name exists
		template < typename String >
		constexpr static auto have_output(String){
			return
				hana::contains(decltype(hana::keys(outputs_))(), String());
		}

		/// \brief hana::true_ if parameter_name exists
		template < typename String >
		constexpr static auto have_parameter(String){
			return
				hana::contains(decltype(hana::keys(parameters_))(), String());
		}


		/// \brief hana::map from input_name's to input's
		Inputs inputs_;

		/// \brief hana::map from output_name's to output's
		Outputs outputs_;

		/// \brief hana::map from parameter_name's to parameter's
		Parameters parameters_;


		/// \brief Type of the exec-function
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		using exec_fn_t =
			std::result_of_t< EnableFunction(module const&) >;

		static_assert(std::is_callable_v< exec_fn_t(module&, std::size_t) >);
#else
		using exec_fn_t =
			std::invoke_result_t< EnableFunction, module const& >;

		static_assert(std::is_invocable_v< exec_fn_t, module&, std::size_t >);
#endif


		/// \brief The function object that is called in enable()
		EnableFunction enable_fn_;

		/// \brief The function object that is called in exec()
		std::optional< exec_fn_t > exec_fn_;
	};


	template <
		typename IOP_MakerList,
		typename EnableFunction >
	struct module_maker{
		/// \brief Tuple of input/output/parameter-maker objects
		IOP_MakerList makers;

		/// \brief The function object that is called in enable()
		EnableFunction enable_fn;


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
						auto const name = maker.name.c_str();
						auto const iter = data.parameters.find(name);
						auto const found = iter != data.parameters.end();

						bool all_specialized = true;

						auto get_value =
							[&data, &all_specialized, found, name, iter]
							(auto type) -> std::optional< std::string_view >
						{
							if(!found) return {};

							auto const specialization = iter->second
								.specialized_values.find(as_text[type].c_str());
							auto const end =
								iter->second.specialized_values.end();
							if(specialization == end){
								all_specialized = false;
								if(!iter->second.generic_value){
									throw std::logic_error(
										data.location() + "parameter '"
										+ std::string(name) + "' has neither a "
										"generic value but a specialization "
										"for type '" + specialization->first
										+ "'"
									);
								}else{
									return {*iter->second.generic_value};
								}
							}else{
								return {specialization->second};
							}
						};

						auto params = hana::to_map(hana::transform(
							maker.types,
							[&get_value](auto&& type){
								return hana::make_pair(type, get_value(type));
							}));

						if(found && all_specialized
							&& iter->second.generic_value
						){
							logsys::log([&data, name](logsys::stdlogb& os){
								os << data.location() << "Warning: parameter '"
									<< name << "' has specialized "
									"values for all its types, the also given "
									"generic value will never be used";
							});
						}

						return hana::append(
							static_cast< decltype(get)&& >(get),
							maker(make_iop_list(get), std::move(params)));
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
					decltype(inputs),
					decltype(outputs),
					decltype(parameters),
					decltype(enable_fn)
				> >(
					data.type_name, data.chain, data.number,
					std::move(inputs),
					std::move(outputs),
					std::move(parameters),
					enable_fn
				);

			return module_ptr;
		}
	};


	template < typename ... IOP_Makers >
	constexpr auto configure(IOP_Makers&& ... list){
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, IOP_Makers >(),
			hana::is_a< output_maker_tag, IOP_Makers >(),
			hana::is_a< parameter_maker_tag, IOP_Makers >()) ...),
			"at least one of the configure arguments is not a disposer input, "
			"output or parameter maker");

		return hana::make_tuple(static_cast< IOP_Makers&& >(list) ...);
	}


	template <
		typename IOP_MakerList,
		typename EnableFunction >
	struct register_fn{
		/// \brief The module_maker object
		module_maker< IOP_MakerList, EnableFunction > maker;

		/// \brief Call this function to register the module with the given type
		///        name via the given module_declarant
		void operator()(std::string const& module_type, module_declarant& add){
			add(module_type, [this](make_data const& data){
				return maker(data);
			});
		}
	};


	template <
		typename IOP_MakerList,
		typename EnableFunction >
	constexpr auto make_register_fn(
		IOP_MakerList&& list,
		EnableFunction&& enable_fn
	){
		return register_fn<
				std::remove_reference_t< IOP_MakerList >,
				std::remove_reference_t< EnableFunction >
			>{{
				static_cast< IOP_MakerList&& >(list),
				static_cast< EnableFunction&& >(enable_fn)
			}};
	}


}


#endif
