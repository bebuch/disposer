//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__generate_module__hpp_INCLUDED_
#define _disposer__core__generate_module__hpp_INCLUDED_

#include "make_module.hpp"

#include "../tool/ct_pretty_name.hpp"


namespace disposer{


	template < bool CanRunConcurrent >
	struct can_run_concurrent: hana::bool_< CanRunConcurrent >{};

	constexpr can_run_concurrent< false > no_overtaking{};


	/// \brief Get help text to the module
	template < typename ... Dimension >
	std::string generate_dims_help(
		dimension_list< Dimension ... > dims
	){
		std::ostringstream help;

		help << "    * dimension count: "
			<< dims.dimension_count << "\n";

		hana::for_each(dims.dimensions,
			[&help, i = 0u](auto const& dim)mutable{
				help << "      * dimension " << ++i << ":\n";
				hana::for_each(dim, [&help](auto type){
						help << "        * " << ct_pretty_name< typename
							decltype(type)::type >() << "\n";
					});
			});

		return help.str();
	}

	/// \brief Get help text to the module
	template < typename ModuleInitFn, typename ExecFn,
		typename ... Dimension, typename ... Config >
	std::string generate_module_help(
		dimension_list< Dimension ... > dims,
		module_configure< Config ... > const& list,
		module_init_fn< ModuleInitFn > const& module_init,
		exec_fn< ExecFn > const& exec
	){
		std::ostringstream help;

		help << generate_dims_help(dims);

		hana::for_each(list.config_list, [&help, dims](auto const& iop){
			auto const is_iop = !hana::is_a< set_dimension_fn_tag >(iop);
			if constexpr(is_iop){
				help << iop.help_text_fn(dims);
			}
		});
// 		help << module_init.help_text;
// 		help << exec.help_text;
		return help.str();
	}


	struct unit_test_key;

	/// \brief Hana Tag for \ref generate_module
	struct generate_module_tag{};

	/// \brief Registers a module configuration in the \ref disposer
	template <
		typename Dimensions,
		typename Configuration,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent >
	class generate_module{
	public:
		/// \brief Hana tag to identify module register functions
		using hana_tag = generate_module_tag;

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		generate_module(
			dimension_list< Dimension ... > dims,
			module_configure< Config ... > list,
			module_init_fn< ModuleInitFn > module_init,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent >
				= can_run_concurrent< true >{}
		)
			: maker_{
				generate_module_help(dims, list, module_init, exec),
				dims,
				std::move(list),
				std::move(module_init),
				std::move(exec)
			} {}

		/// \brief Constructor
		template < typename ... Dimension >
		generate_module(
			dimension_list< Dimension ... > dims,
			module_init_fn< ModuleInitFn > module_init,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dims, module_configure<>{},
				std::move(module_init), std::move(exec), crc)
			{}

		/// \brief Constructor
		template < typename ... Dimension, typename ... Config >
		generate_module(
			dimension_list< Dimension ... > dims,
			module_configure< Config ... > list,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dims, std::move(list),
				module_init_fn< void >(), std::move(exec), crc)
			{}

		/// \brief Constructor
		template < typename ... Dimension >
		generate_module(
			dimension_list< Dimension ... > dims,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dims, module_configure<>{},
				module_init_fn< void >(), std::move(exec), crc)
			{}

		/// \brief Constructor
		template < typename ... Config >
		generate_module(
			module_configure< Config ... > list,
			module_init_fn< ModuleInitFn > module_init,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dimension_list{}, std::move(list),
				std::move(module_init), std::move(exec), crc)
			{}

		/// \brief Constructor
		generate_module(
			module_init_fn< ModuleInitFn > module_init,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dimension_list{}, module_configure<>{},
				std::move(module_init), std::move(exec), crc)
			{}

		/// \brief Constructor
		template < typename ... Config >
		generate_module(
			module_configure< Config ... > list,
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dimension_list{}, std::move(list),
				module_init_fn< void >(), std::move(exec), crc)
			{}

		/// \brief Constructor
		generate_module(
			exec_fn< ExecFn > exec,
			can_run_concurrent< CanRunConcurrent > crc
				= can_run_concurrent< true >{}
		)
			: generate_module(dimension_list{}, module_configure<>{},
				module_init_fn< void >(), std::move(exec), crc)
			{}


		/// \brief Generates help text
		std::string help(std::string const& module_type)const{
			std::ostringstream help;
			help << "  * module: " << module_type << "\n";
			help << maker_.help_text << "\n";
			return help.str();
		}


		/// \brief Call this function to register the module with the given type
		///        name via the given module_declarant
		void operator()(
			std::string const& module_type,
			module_declarant& add
		)const{
			add(module_type, module_maker_entry{
				[maker = maker_](module_make_data const& data){
					return maker(data);
				},
				help(module_type)});
		}


		/// \brief Call this function to register the module with the given type
		///        name and an existing component via the given module_declarant
		template < typename Component >
		void operator()(
			std::string const& module_type,
			module_declarant& add,
			Component& component
		)const{
			add(module_type, module_maker_entry{
				[maker = maker_, &component]
				(module_make_data const& data){
					return maker(data, component);
				},
				help(module_type)});
		}



	private:
		/// \brief The module_maker object
		module_maker< Dimensions, Configuration, ModuleInitFn, ExecFn,
			CanRunConcurrent > maker_;

		friend struct unit_test_key;
	};


	template <
		typename ... Dimension,
		typename ... Config,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		dimension_list< Dimension ... > dims,
		module_configure< Config ... > list,
		module_init_fn< ModuleInitFn > module_init,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list< Dimension ... >,
			module_configure< Config ... >,
			ModuleInitFn, ExecFn, CanRunConcurrent >;

	template <
		typename ... Dimension,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		dimension_list< Dimension ... > dims,
		module_init_fn< ModuleInitFn > module_init,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list< Dimension ... >,
			module_configure<>,
			ModuleInitFn, ExecFn, CanRunConcurrent >;

	template <
		typename ... Dimension,
		typename ... Config,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		dimension_list< Dimension ... > dims,
		module_configure< Config ... > list,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list< Dimension ... >,
			module_configure< Config ... >,
			void, ExecFn, CanRunConcurrent >;

	template <
		typename ... Dimension,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		dimension_list< Dimension ... > dims,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list< Dimension ... >,
			module_configure<>,
			void, ExecFn, CanRunConcurrent >;

	template <
		typename ... Config,
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		module_configure< Config ... > list,
		module_init_fn< ModuleInitFn > module_init,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list<>,
			module_configure< Config ... >,
			ModuleInitFn, ExecFn, CanRunConcurrent >;

	template <
		typename ModuleInitFn,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		module_init_fn< ModuleInitFn > module_init,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list<>,
			module_configure<>,
			ModuleInitFn, ExecFn, CanRunConcurrent >;

	template <
		typename ... Config,
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		module_configure< Config ... > list,
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list<>,
			module_configure< Config ... >,
			void, ExecFn, CanRunConcurrent >;

	template <
		typename ExecFn,
		bool CanRunConcurrent = true >
	generate_module(
		exec_fn< ExecFn > exec,
		can_run_concurrent< CanRunConcurrent > = can_run_concurrent< true >{}
	) -> generate_module<
			dimension_list<>,
			module_configure<>,
			void, ExecFn, CanRunConcurrent >;


}


#endif
