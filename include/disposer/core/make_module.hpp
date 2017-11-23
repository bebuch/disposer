//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_module__hpp_INCLUDED_
#define _disposer__core__make_module__hpp_INCLUDED_

#include "module.hpp"
#include "make_input.hpp"
#include "make_output.hpp"
#include "make_parameter.hpp"
#include "set_dimension_fn.hpp"
#include "verify_fn.hpp"

#include "../config/validate_iop.hpp"

#include "../tool/config_queue.hpp"


namespace disposer{


	/// \brief The list of module configuration items
	template < typename ... Config >
	struct module_configure{
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, Config >(),
			hana::is_a< output_maker_tag, Config >(),
			hana::is_a< parameter_maker_tag, Config >(),
			hana::is_a< set_dimension_fn_tag, Config >()) ...),
			"at least one of the module_configure arguments is not "
			"an input maker, an output maker, a parameter maker or "
			"a set_dimension_fn");

		/// \brief The data
		hana::tuple< Config ... > config_list;

		/// \brief Constructor
		constexpr module_configure(Config ... list)
			: config_list(std::move(list) ...) {}
	};


	/// \brief Deduction guide to store all config items as values
	template < typename ... Config >
	module_configure(Config&& ...) -> module_configure
		< std::remove_cv_t< std::remove_reference_t< Config > > ... >;


	template <
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	struct module_construction{
		module_make_data const& data;
		module_init_fn< ModuleInitFn > const& module_init;
		exec_fn< ExecFn > const& exec;
		optional_component< Component > component;

		template < typename DimensionList >
		struct input_construction{
			module_construction< ModuleInitFn, ExecFn, CanRunConcurrent,
				Component > const& base;

			template <
				typename Name,
				typename DimensionReferrer,
				bool IsRequired,
				std::size_t Offset,
				typename ... Config,
				typename ... IOPs,
				std::size_t ... SDs >
			std::unique_ptr< module_base > make(
				input_maker< Name, DimensionReferrer, IsRequired > const& maker,
				detail::config_queue< Offset, Config ... > const configs,
				iops_ref< IOPs ... >&& iops,
				solved_dimensions< SDs ... > const& solved_dims,
				output_base* const output_ptr
			)const{
				if constexpr(!solved_dimensions< SDs ... >::is_empty()){
					auto const current_dim_number =
						solved_dims.dimension_number();

					using dim_type = decltype(current_dim_number);

					using make_fn_type =
						std::unique_ptr< module_base >(*)(
							module_construction< ModuleInitFn, ExecFn,
								CanRunConcurrent, Component > const&,
							input_maker< Name, DimensionReferrer, IsRequired >
								const&,
							detail::config_queue< Offset, Config ... > const,
							iops_ref< IOPs ... >&&,
							decltype(solved_dims.rest()) const&,
							output_base* const
						);

					constexpr auto generate_next = [](auto i){
							using index_type = decltype(i);
							return [](
									module_construction< ModuleInitFn, ExecFn,
										CanRunConcurrent, Component >
											const& base,
									input_maker< Name, DimensionReferrer,
										IsRequired > const& maker,
									detail::config_queue< Offset, Config ... >
										const configs,
									iops_ref< IOPs ... >&& iops,
									decltype(solved_dims.rest()) const&
										solved_dims,
									output_base* const output_ptr
								){
									auto next_dimension_list =
										reduce_dimension_list(DimensionList{},
											ct_index_component<
												dim_type::value,
												index_type::value >{});

									return input_construction<
										decltype(next_dimension_list) >{base}
										.make(maker, configs, std::move(iops),
											solved_dims, output_ptr);
								};
						};

					constexpr auto types =
						DimensionList::dimensions[current_dim_number];
					constexpr auto tc = hana::size(types).value;

					constexpr auto call = hana::unpack(
						hana::range_c< std::size_t, 0, tc >,
						[generate_next](auto ... i){
							return std::array< make_fn_type, sizeof...(i) >{{
									generate_next(i) ...
								}};
						});

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
					assert(solved_dims.index_number() < tc);
#endif

					return call[solved_dims.index_number()](
							base,
							maker,
							configs,
							std::move(iops),
							solved_dims.rest(),
							output_ptr
						);
				}else{
					(void)solved_dims; // Silance GCC

					DimensionReferrer::verify_solved(DimensionList{});

					using type = typename
						DimensionReferrer::template type< DimensionList >;

					if constexpr(std::is_void_v< type >){
						return base.make_module(DimensionList{}, configs,
							std::move(iops));
					}else{
						if(IsRequired || output_ptr != nullptr){
							constexpr auto active_ti
								= type_index::type_id< type >();
							auto const output_ti = output_ptr->get_type();

							if(output_ti != active_ti){
								throw std::logic_error("type of input is ["
									+ active_ti.pretty_name()
									+ "] but connected output is of type ["
									+ output_ti.pretty_name() + "]");
							}
						}

						input< Name, type, IsRequired > input{output_ptr};

						return base.make_module(DimensionList{}, configs,
							iops_ref(std::move(input), std::move(iops)));
					}
				}
			}
		};

		template <
			typename Name,
			typename DimensionReferrer,
			bool IsRequired,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_make_input(
			input_maker< Name, DimensionReferrer, IsRequired > const& maker,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			auto const output_ptr = get_output_ptr(data.inputs,
				detail::to_std_string_view(Name{}));

			if constexpr(IsRequired){
				if(output_ptr == nullptr){
					throw std::logic_error("input is required but not set");
				}
			}

			input_construction< dimension_list< Ds ... > > const base{*this};

			return base.make(maker, configs, std::move(iops),
				deduce_dimensions_by_input(maker, dims, output_ptr),
				output_ptr);
		}

		template < typename DimensionList >
		struct set_dimension_fn_execution{
			module_construction< ModuleInitFn, ExecFn,
				CanRunConcurrent, Component > const& base;

			template <
				std::size_t Offset,
				typename ... Config,
				typename ... IOPs,
				std::size_t ... SDs >
			std::unique_ptr< module_base > make(
				detail::config_queue< Offset, Config ... > const configs,
				iops_ref< IOPs ... >&& iops,
				solved_dimensions< SDs ... > const& solved_dims
			)const{
				if constexpr(!solved_dimensions< SDs ... >::is_empty()){
					constexpr auto current_dim_number =
						solved_dimensions< SDs ... >::dimension_number();

					using make_fn_type =
						std::unique_ptr< module_base >(*)(
							module_construction< ModuleInitFn, ExecFn,
								CanRunConcurrent, Component > const&,
							detail::config_queue< Offset, Config ... > const,
							iops_ref< IOPs ... >&&,
							decltype(solved_dims.rest()) const&
						);

					constexpr auto generate_next = [](auto d, auto i){
							using dim_type = decltype(d);
							using index_type = decltype(i);
							return [](
									module_construction< ModuleInitFn, ExecFn,
										CanRunConcurrent, Component >
											const& base,
									detail::config_queue< Offset, Config ... >
										const configs,
									iops_ref< IOPs ... >&& iops,
									decltype(solved_dims.rest()) const&
										solved_dims
								){
									auto next_dimension_list =
										reduce_dimension_list(DimensionList{},
											ct_index_component<
												dim_type::value,
												index_type::value >{});

									return set_dimension_fn_execution<
										decltype(next_dimension_list) >{base}
										.make(configs, std::move(iops),
											solved_dims);
								};
						};

					constexpr auto types =
						DimensionList::dimensions[current_dim_number];
					constexpr auto tc = hana::size(types).value;

					constexpr auto call = hana::unpack(
						hana::range_c< std::size_t, 0, tc >,
						[generate_next, current_dim_number](auto ... i){
							return std::array< make_fn_type, sizeof...(i) >{{
									generate_next(current_dim_number, i) ...
								}};
						});

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
					assert(solved_dims.index_number() < tc);
#endif

					return call[solved_dims.index_number()](
							base,
							configs,
							std::move(iops),
							solved_dims.rest()
						);
				}else{
					(void)solved_dims; // Silance GCC

					return base.make_module(
						DimensionList{}, configs, std::move(iops));
				}
			}
		};

		template <
			typename Name,
			typename DimensionReferrer,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_make_output(
			output_maker< Name, DimensionReferrer > const&,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			DimensionReferrer::verify_solved(dims);

			using type = typename
				DimensionReferrer::template type< dimension_list< Ds ... > >;

			if constexpr(std::is_void_v< type >){
				return make_module(dims, configs, std::move(iops));
			}else{
				auto const use_count = get_use_count(data.outputs,
					detail::to_std_string_view(Name{}));

				output< Name, type > output{use_count};

				return make_module(dims, configs,
					iops_ref(std::move(output), std::move(iops)));
			}
		}

		template <
			typename Name,
			typename DimensionReferrer,
			typename ParserFn,
			typename DefaultValueFn,
			typename VerfiyValueFn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_make_parameter(
			parameter_maker< Name, DimensionReferrer,
				ParserFn, DefaultValueFn, VerfiyValueFn > const& maker,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			DimensionReferrer::verify_solved(dims);

			using type = typename
				DimensionReferrer::template type< dimension_list< Ds ... > >;

			if constexpr(std::is_void_v< type >){
				return make_module(dims, configs, std::move(iops));
			}else{
				auto const param_data_ptr = get_parameter_data(data.parameters,
					detail::to_std_string_view(Name{}));

				parameter< Name, type > parameter{get_parameter_value< type >(
						detail::to_std_string_view(maker.name),
						dims,
						maker.parser,
						maker.default_value_generator,
						maker.verify_value,
						module_make_accessory{
							component, dims, iops, data.location()},
						param_data_ptr
					)};

				return make_module(dims, configs,
					iops_ref(std::move(parameter), std::move(iops)));
			}
		}

		template <
			typename Fn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_verify_fn(
			verify_fn< Fn > const& fn,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			fn(module_make_accessory{component, dims, iops, data.location()});
			return make_module(dims, configs, std::move(iops));
		}

		template <
			typename Fn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_set_dimension_fn(
			set_dimension_fn< Fn > const& fn,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			set_dimension_fn_execution< dimension_list< Ds ... > > const
				base{*this};

			return base.make(configs, std::move(iops), fn(
				module_make_accessory{component, dims, iops, data.location()}));
		}


		template <
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > make_module(
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			if constexpr(detail::config_queue< Offset, Config ... >::is_empty){
				(void)configs; // Silance GCC;

				return std::unique_ptr< module_base >(new module{
					type_list{dims},
					data.chain, data.type_name, data.number,
					std::move(iops).flat(), module_init, exec,
					hana::bool_c< CanRunConcurrent >, component});
			}else{
				using hana::is_a;
				auto const& config = configs.front();

				if constexpr(
					auto c = is_a< input_maker_tag >(config); c
				){
					return exec_make_input(config, dims, configs.next(),
						std::move(iops));
				}else if constexpr(
					auto c = is_a< output_maker_tag >(config); c
				){
					return exec_make_output(config, dims, configs.next(),
						std::move(iops));
				}else if constexpr(
					auto c = is_a< parameter_maker_tag >(config); c
				){
					return exec_make_parameter(config, dims, configs.next(),
						std::move(iops));
				}else if constexpr(
					auto c = is_a< verify_fn_tag >(config); c
				){
					return exec_verify_fn(config, dims, configs.next(),
						std::move(iops));
				}else{
					auto is_set_dimension_fn =
						is_a< set_dimension_fn_tag >(config);
					static_assert(is_set_dimension_fn);
					return exec_set_dimension_fn(config, dims, configs.next(),
						std::move(iops));
				}
			}
		}
	};

	/// \brief Maker function for \ref module in a std::unique_ptr
	template <
		typename ... Ds,
		typename ... Config,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent,
		typename Component >
	std::unique_ptr< module_base > make_module_ptr(
		dimension_list< Ds ... > dims,
		module_configure< Config ... > const& configs,
		module_make_data const& data,
		module_init_fn< ModuleInitFn > const& module_init,
		exec_fn< ExecFn > const& exec,
		hana::bool_< CanRunConcurrent >,
		optional_component< Component > component
	){
		detail::config_queue const queue{configs.config_list};
		module_construction< ModuleInitFn, ExecFn, CanRunConcurrent,
			Component > const mc{data, module_init, exec, component};
		return mc.make_module(dims, queue, iops_ref{});
	}


	/// \brief Check config file data for undefined inputs, outputs and
	///        parameters
	///
	/// Warn about parameters, throw for inputs and outputs.
	template < typename Configuration >
	void validate_iops(
		Configuration const& configuration,
		module_make_data const& data
	){
		auto const location = data.location();
		auto const inputs = validate_iop< input_maker_tag >(
			location, configuration, data.inputs);
		auto const outputs = validate_iop< output_maker_tag >(
			location, configuration, data.outputs);
		validate_iop< parameter_maker_tag >(
			location, configuration, data.parameters);

		if(!inputs.empty() || !outputs.empty()){
			throw std::logic_error(location + "some inputs or "
				"outputs don't exist, see previos log messages for "
				"more details");
		}
	}

	/// \brief Tag to identify module_maker objects via hana::is_a
	struct module_maker_tag;

	/// \brief Provids types for constructing an module
	template <
		typename DimensionList,
		typename Configuration,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent >
	struct module_maker{
		/// \brief Hana tag
		using hana_tag = module_maker_tag;


		/// \brief An dimension_list object
		DimensionList dimensions;

		/// \brief Tuple of input/output/parameter-maker objects
		Configuration configuration;

		/// \brief The function object that is called in enable()
		module_init_fn< ModuleInitFn > module_init;

		/// \brief The function object that is called in exec()
		exec_fn< ExecFn > exec;


		/// \brief Create an module object
		std::unique_ptr< module_base > operator()(
			module_make_data const& data
		)const{
			validate_iops(configuration, data);

			return make_module_ptr(dimensions, configuration, data,
				module_init, exec, hana::bool_c< CanRunConcurrent >,
				optional_component< void >{});
		}

		/// \brief Create an module object bound to an existing component object
		template < typename Component >
		std::unique_ptr< module_base > operator()(
			module_make_data const& data,
			Component& component
		)const{
			validate_iops(configuration, data);

			return make_module_ptr(dimensions, configuration, data,
				module_init, exec, hana::bool_c< CanRunConcurrent >,
				optional_component{component});
		}
	};


}


#endif
