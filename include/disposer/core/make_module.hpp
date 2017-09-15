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

#include "../config/validate_iop.hpp"
#include "../config/module_make_data.hpp"

#include "../tool/config_queue.hpp"

#include <atomic>


namespace disposer{


	/// \brief The list of module configuration items
	template < typename ... Config >
	struct module_configure{
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< input_maker_tag, Config >(),
			hana::is_a< output_maker_tag, Config >(),
			hana::is_a< parameter_maker_tag, Config >(),
			hana::is_a< set_dimension_fn_tag, Config >()) ...),
			"at least one of the module configure arguments is not "
			"an input maker, an output maker, a parameter maker or "
			"a set_dimension_fn");

		/// \brief The data
		hana::tuple< Config ... > config_list;

		/// \brief Constructor
		constexpr module_configure(Config&& ... list)
			: config_list(static_cast< Config&& >(list) ...) {}
	};


	/// \brief Deduction guide to store all config items as values
	template < typename ... Config >
	module_configure(Config&& ...) -> module_configure
		< std::remove_cv_t< std::remove_reference_t< Config > > ... >;


	template < typename StateMakerFn, typename ExecFn >
	struct module_construction{
		module_make_data const& data;
		state_maker_fn< StateMakerFn > const& state_maker;
		exec_fn< ExecFn > const& exec;

		template < typename DimensionList >
		struct input_construction{
			module_construction< StateMakerFn, ExecFn > const& base;

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
				detail::config_queue< Offset, Config ... > const& configs,
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
							module_construction< StateMakerFn, ExecFn > const&,
							input_maker< Name, DimensionReferrer, IsRequired >
								const&,
							detail::config_queue< Offset, Config ... > const&,
							iops_ref< IOPs ... >&&,
							decltype(solved_dims.rest()) const&,
							output_base* const
						);

					constexpr auto generate_next = [](auto i){
							using index_type = decltype(i);
							return [](
									module_construction< StateMakerFn, ExecFn >
										const& base,
									input_maker< Name, DimensionReferrer,
										IsRequired > const& maker,
									detail::config_queue< Offset, Config ... >
										const& configs,
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
										.next(
											maker, configs, std::move(iops),
											solved_dims, output_ptr
										);
								};
						};

					constexpr auto tc = DimensionList
						::dimensions[current_dim_number].type_count;

					constexpr auto call = hana::unpack(
						hana::range_c< std::size_t, 0, tc >,
						[generate_next](auto ... i){
							return std::array< make_fn_type, sizeof...(i) >{{
									generate_next(i) ...
								}};
						});

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
			dimension_list< Ds ... > const& dims,
			detail::config_queue< Offset, Config ... > const& configs,
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
			module_construction< StateMakerFn, ExecFn > const& base;

			template <
				std::size_t Offset,
				typename ... Config,
				typename ... IOPs,
				std::size_t ... SDs >
			std::unique_ptr< module_base > make(
				detail::config_queue< Offset, Config ... > const& configs,
				iops_ref< IOPs ... >&& iops,
				solved_dimensions< SDs ... > const& solved_dims
			)const{
				if constexpr(!solved_dimensions< SDs ... >::is_empty()){
					constexpr auto current_dim_number =
						solved_dimensions< SDs ... >::dimension_number();

					using make_fn_type =
						std::unique_ptr< module_base >(*)(
							module_construction< StateMakerFn, ExecFn > const&,
							detail::config_queue< Offset, Config ... > const&,
							iops_ref< IOPs ... >&&,
							decltype(solved_dims.rest()) const&
						);

					constexpr auto generate_next = [](auto d, auto i){
							using dim_type = decltype(d);
							using index_type = decltype(i);
							return [](
									module_construction< StateMakerFn, ExecFn >
										const& base,
									detail::config_queue< Offset, Config ... >
										const& configs,
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
			typename Fn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > exec_set_dimension_fn(
			set_dimension_fn< Fn > const& fn,
			dimension_list< Ds ... > const& dims,
			detail::config_queue< Offset, Config ... > const& configs,
			iops_ref< IOPs ... >&& iops
		)const{
			set_dimension_fn_execution< dimension_list< Ds ... > > const
				base{*this};

			return base.make(configs, std::move(iops),
				fn(dims, iops_accessory{iops, data.location()}));
		}


		template <
			typename ... Dimension,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< module_base > make_module(
			dimension_list< Dimension ... > const& dims,
			detail::config_queue< Offset, Config ... > const& configs,
			iops_ref< IOPs ... >&& iops
		)const{
			if constexpr(detail::config_queue< Offset, Config ... >::is_empty){
				(void)configs; // Silance GCC;
				return std::unique_ptr< module_base >(new module{
					data.chain, data.type_name, data.number,
					std::move(iops).flat(), state_maker, exec});

			}else{
				using hana::is_a;
				auto const& config = configs.front();

				if constexpr(auto c = is_a< input_maker_tag >(config); c){
					return exec_make_input(config, dims, configs.next(),
						std::move(iops));
	// 			}else if constexpr(auto c = is_a< output_maker_tag >(config); c){
	// 				return exec_make_output(config, dims, configs.next(),
	// 					std::move(iops));
	// 			}else if constexpr(auto c = is_a< parameter_maker_tag >(config); c){
	// 				return exec_make_parameter(config, dims, configs.next(),
	// 					std::move(iops));
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
		typename ... Dimension,
		typename ... Config,
		typename StateMakerFn,
		typename ExecFn >
	std::unique_ptr< module_base > make_module_ptr(
		dimension_list< Dimension ... > const& dims,
		module_configure< Config ... > const& configs,
		module_make_data const& data,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	){
		detail::config_queue queue{configs.config_list};
		module_construction< StateMakerFn, ExecFn > const mc
			{data, state_maker, exec};
		return mc.make_module(dims, queue, iops_ref{});
	}


	/// \brief Provids types for constructing an module
	template <
		typename DimensionList,
		typename Configuration,
		typename StateMakerFn,
		typename ExecFn >
	struct module_maker{
		/// \brief An dimension_list object
		DimensionList dimensions;

		/// \brief Tuple of input/output/parameter-maker objects
		Configuration configuration;

		/// \brief The function object that is called in enable()
		state_maker_fn< StateMakerFn > state_maker;

		/// \brief The function object that is called in exec()
		exec_fn< ExecFn > exec;


		/// \brief Create an module object
		std::unique_ptr< module_base > operator()(
			module_make_data const& data
		)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto const location = data.location();
			{
				auto inputs = validate_iop< input_maker_tag >(
					location, configuration, data.inputs);
				auto outputs = validate_iop< output_maker_tag >(
					location, configuration, data.outputs);
				validate_iop< parameter_maker_tag >(
					location, configuration, data.parameters);

				if(!inputs.empty() || !outputs.empty()){
					throw std::logic_error(location + "some inputs or "
						"outputs don't exist, see previos log messages for "
						"more details");
				}
			}

			// Create the module
			return make_module_ptr(configuration, data, state_maker, exec);
		}
	};

	struct unit_test_key;

	/// \brief Hana Tag for \ref module_register_fn
	struct module_register_fn_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename Dimensions,
		typename Configuration,
		typename StateMakerFn,
		typename ExecFn >
	class module_register_fn{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = module_register_fn_tag;

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		module_register_fn(
			dimension_list< Dimension ... > const& dims,
			module_configure< Config ... >&& list,
			state_maker_fn< StateMakerFn >&& state_maker,
			exec_fn< ExecFn >&& exec
		)
			: called_flag_(false)
			, maker_{dims, std::move(list), state_maker, exec}
			{}

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		module_register_fn(
			dimension_list< Dimension ... > const& dims,
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dims, std::move(list),
				state_maker_fn< void >(), std::move(exec))
			{}

		/// \brief Constructor
		template < typename ... Config >
		module_register_fn(
			module_configure< Config ... >&& list,
			state_maker_fn< StateMakerFn >&& state_maker,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				std::move(state_maker), std::move(exec))
			{}

		/// \brief Constructor
		template < typename ... Config >
		module_register_fn(
			module_configure< Config ... >&& list,
			exec_fn< ExecFn >&& exec
		)
			: module_register_fn(dimension_list{}, std::move(list),
				state_maker_fn< void >(), std::move(exec))
			{}

		/// \brief Call this function to register the module with the given type
		///        name via the given module_declarant
		void operator()(std::string const& module_type, module_declarant& add){
			if(!called_flag_.exchange(true)){
				add(module_type,
					[maker{std::move(maker_)}](module_make_data const& data){
						return maker(data);
					});
			}else{
				throw std::runtime_error("called module register function '"
					+ module_type + "' more than once");
			}
		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The module_maker object
		module_maker< Dimensions, Configuration, StateMakerFn, ExecFn > maker_;

		friend struct unit_test_key;
	};


	/// \brief Deduction guide
	template <
		typename StateMakerFn,
		typename ExecFn,
		typename ... Dimension,
		typename ... Config >
	module_register_fn(
		dimension_list< Dimension ... > const& dims,
		module_configure< Config ... >&& list,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	) -> module_register_fn<
		dimension_list< Dimension ... >,
		module_configure< Config ... >,
		StateMakerFn, ExecFn >;

}


#endif
