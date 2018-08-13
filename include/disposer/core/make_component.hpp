//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_component__hpp_INCLUDED_
#define _disposer__core__make_component__hpp_INCLUDED_

#include "component.hpp"
#include "make_module.hpp"

#include "set_dimension_fn.hpp"

#include "../config/validate_iop.hpp"

#include "../tool/config_queue.hpp"

#include <atomic>



namespace disposer{



	/// \brief The list of component configuration items
	template < typename ... Config >
	struct component_configure{
		static_assert(hana::and_(hana::true_c, hana::or_(
			hana::is_a< parameter_maker_tag, Config >(),
			hana::is_a< set_dimension_fn_tag, Config >(),
			hana::is_a< verify_fn_tag , Config >()) ...),
			"at least one of the component_configure arguments is not "
			"a parameter maker, a set_dimension_fn or a verify_fn");

		/// \brief The data
		hana::tuple< Config ... > config_list;

		/// \brief Constructor
		constexpr component_configure(Config ... list)
			: config_list(std::move(list) ...) {}
	};


	/// \brief Deduction guide to store all config items as values
	template < typename ... Config >
	component_configure(Config&& ...) -> component_configure
		< std::remove_cv_t< std::remove_reference_t< Config > > ... >;


	/// \brief The list of component modules
	template < typename ... Module >
	struct component_modules{
		static_assert(hana::and_(hana::true_c,
			hana::is_a< component_module_maker_tag, Module >() ...),
			"at least one of the component_modules arguments is not a "
			"module_name");

		/// \brief The data
		hana::tuple< Module ... > module_maker_list;

		/// \brief Constructor
		constexpr component_modules(Module ... list)
			: module_maker_list(std::move(list) ...) {}
	};


	/// \brief Deduction guide to store all config items as values
	template < typename ... Module >
	component_modules(Module ...) -> component_modules
		< std::remove_cv_t< std::remove_reference_t< Module > > ... >;


	template < typename ComponentInitFn, typename Modules >
	struct component_construction{
		disposer::system& system;

		component_make_data const& data;

		/// \brief The function that creates the state object
		component_init_fn< ComponentInitFn > const& component_init;

		/// \brief Tuple of module_maker objects
		Modules module_maker_list;


		template < typename DimensionList >
		struct set_dimension_fn_execution{
			component_construction< ComponentInitFn, Modules > const& base;

			template <
				std::size_t Offset,
				typename ... Config,
				typename ... IOPs,
				std::size_t ... SDs >
			std::unique_ptr< component_base > make(
				detail::config_queue< Offset, Config ... > const configs,
				iops_ref< IOPs ... >&& iops,
				solved_dimensions< SDs ... > const& solved_dims
			)const{
				if constexpr(!solved_dimensions< SDs ... >::is_empty()){
					constexpr auto current_dim_number =
						solved_dimensions< SDs ... >::dimension_number();

					using make_fn_type =
						std::unique_ptr< component_base >(*)(
							component_construction< ComponentInitFn,
								Modules > const&,
							detail::config_queue< Offset, Config ... > const,
							iops_ref< IOPs ... >&&,
							decltype(solved_dims.rest()) const&
						);

					constexpr auto generate_next = [](auto d, auto i){
							using dim_type = decltype(d);
							using index_type = decltype(i);
							return [](
									component_construction< ComponentInitFn,
										Modules > const& base,
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

					return base.make_component(
						DimensionList{}, configs, std::move(iops));
				}
			}
		};

		template <
			typename Name,
			typename DimensionReferrer,
			typename ParserFn,
			typename DefaultValueFn,
			typename DefaultValueHelpFn,
			typename VerfiyValueFn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< component_base > exec_make_parameter(
			parameter_maker< Name, DimensionReferrer, ParserFn,
					DefaultValueFn, DefaultValueHelpFn, VerfiyValueFn
				> const& maker,
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
						component_make_ref{dims, iops, data.location()},
						param_data_ptr
					)};

				return make_component(dims, configs,
					iops_ref(std::move(parameter), std::move(iops)));
			}
		}

		template <
			typename Fn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< component_base > exec_verify_fn(
			verify_fn< Fn > const& fn,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			fn(component_make_ref{dims, iops, data.location()});
			return make_component(dims, configs, std::move(iops));
		}

		template <
			typename Fn,
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< component_base > exec_set_dimension_fn(
			set_dimension_fn< Fn > const& fn,
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			set_dimension_fn_execution< dimension_list< Ds ... > > const
				base{*this};

			return base.make(configs, std::move(iops),
				fn(component_make_ref{dims, iops, data.location()}));
		}


		template <
			typename ... Ds,
			std::size_t Offset,
			typename ... Config,
			typename ... IOPs >
		std::unique_ptr< component_base > make_component(
			dimension_list< Ds ... > dims,
			detail::config_queue< Offset, Config ... > const configs,
			iops_ref< IOPs ... >&& iops
		)const{
			if constexpr(detail::config_queue< Offset, Config ... >::is_empty){
				(void)configs; // Silance GCC;

				// Template arguments of unique_ptr's can't be deduced,
				// therefor create the object unmanged but with full type
				// information then immediately transfer the ownership to a
				// unique_ptr of the base class
				auto const component_ptr = new component{
					type_list{dims}, data.name, data.type_name, system,
					std::move(iops).flat(), component_init};

				std::unique_ptr< component_base > result(component_ptr);
				auto& component = *component_ptr;

				// register the component modules for this component instance
				hana::for_each(module_maker_list.module_maker_list,
					[this, &component](auto const& component_module_maker){
						component_module_maker.generate_module_fn(
							std::string(component_module_maker.name),
							system.directory().declarant(), component);
					});

				return result;
			}else{
				using hana::is_a;
				auto const& config = configs.front();

				if constexpr(
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


	/// \brief Maker function for \ref component in a std::unique_ptr
	template <
		typename ... Ds,
		typename ... Config,
		typename ... Module,
		typename ComponentInitFn >
	std::unique_ptr< component_base > make_component_ptr(
		dimension_list< Ds ... > dims,
		component_configure< Config ... > const& configs,
		component_modules< Module ... > const& module_maker_list,
		disposer::system& system,
		component_make_data const& data,
		component_init_fn< ComponentInitFn > const& component_init
	){
		detail::config_queue const queue{configs.config_list};
		component_construction<
			ComponentInitFn, component_modules< Module ... > > const mc
			{system, data, component_init, module_maker_list};
		return mc.make_component(dims, queue, iops_ref{});
	}

	/// \brief Provids types for constructing an component
	template <
		typename DimensionList,
		typename Configuration,
		typename Modules,
		typename ComponentInitFn >
	struct component_maker{
		/// \brief Description of the component and its modules
		std::string help_text_fn(std::string const& component_type)const{
			std::ostringstream help;
			help << "  * component: " << component_type << "\n";
			help << help_text << "\n";
			hana::for_each(configuration.config_list, [&help](auto const& iop){
					auto const is_iop = hana::is_a< parameter_tag >(iop);
					if constexpr(is_iop){
						help << iop.help_text_fn(DimensionList{});
					}
				});
			hana::for_each(module_maker_list.module_maker_list,
				[&component_type, &help](auto const& module_maker){
					help << module_maker.generate_module_fn.help_text_fn(
						"[" + component_type + "]//"
						+ std::string(module_maker.name));
				});
			return help.str();
		}

		/// \brief An dimension_list object
		DimensionList dimensions;

		/// \brief Tuple of parameter-maker objects
		Configuration configuration;

		/// \brief Tuple of module_maker objects
		Modules module_maker_list;

		/// \brief The function that creates the state object
		component_init_fn< ComponentInitFn > component_init;

		/// \brief User defined help text
		std::string const help_text;


		/// \brief Create an component object
		auto operator()(
			component_make_data const& data,
			disposer::system& system
		)const{
			// Check config file data for undefined inputs, outputs and
			// parameters, warn about parameters, throw for inputs and outputs
			auto const location = data.location();
			validate_iop< parameter_maker_tag >(
				location, configuration, data.parameters);

			// Create the component
			return make_component_ptr(dimensions, configuration,
				module_maker_list, system, data, component_init);
		}
	};


	/// \brief Calculate the component_init_ref type for components
	///        without dimensions
	///
	/// If your component has dimensions the function will fail by a
	/// static_assert.
	template < typename ... Config >
	constexpr auto parameter_tuple_of(
		hana::basic_type< component_configure< Config ... > >
	)noexcept{
		static_assert(hana::and_(hana::true_c, hana::and_(
				hana::is_a< parameter_maker_tag, Config >(),
				Config::is_free_type) ...),
			"init_ref_of requires all component_configure entries "
			"to be parameters with a free_type_c");

		return hana::type_c< hana::tuple< config_t< Config > ... > >;
	}


	/// \brief Type of init_ref for components without dimensions
	template < typename Configure >
	using component_init_ref_t = component_init_ref< type_list<>,
		typename decltype(parameter_tuple_of(
			hana::type_c< Configure >))::type >;

	/// \brief Type of ref for components without dimensions
	template < typename Configure, typename StateType >
	using component_ref_t = component_ref< type_list<>, StateType,
		typename decltype(parameter_tuple_of(
			hana::type_c< Configure >))::type >;


}


#endif
