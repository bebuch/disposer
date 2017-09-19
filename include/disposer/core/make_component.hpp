//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_component__hpp_INCLUDED_
#define _disposer__core__make_component__hpp_INCLUDED_

#include "component_base.hpp"
#include "module_name.hpp"

#include "make_parameter.hpp"
#include "module_init_fn.hpp"

#include "../config/validate_iop.hpp"
#include "../config/component_make_data.hpp"

#include "../tool/config_queue.hpp"

#include <atomic>



namespace disposer{


	/// \brief Accessory of a \ref component without log
	template < typename ... Parameters >
	class component_config{
	public:
		/// \brief Constructor
		template < typename ... RefList >
		component_config(iops_ref< RefList ... > const& ref_list)
			, parameters_(move_parameters(ref_list)) {}


		/// \brief Get reference to a parameter-object via its corresponding
		///        compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return extract(list_, name);
		}

		/// \brief Get reference to a parameter-object via its corresponding
		///        compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return extract(list_, name);
		}


	private:
		template < typename L, typename Name >
		static auto& extract(L& list, Name const& name)noexcept{
			using name_t = std::remove_reference_t< Name >;
			static_assert(hana::is_a< parameter_name_tag, name_t >,
				"name is not a parameter_name");
			return detail::extract(detail::as_ref_list(list), name);
		}


		/// \brief hana::tuple of the inputs, outputs and parameters
		List list_;
	};


	/// \brief Accessory of a component
	template < typename ... Parameters >
	class component_accessory
		: public component_config< Parameters ... >
		, public add_log< component_accessory< Parameters ... > >
	{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		component_accessory(
			component_base& component,
			::disposer::disposer& disposer,
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location
		)
			: component_config< List >(
					maker_list, data, location, std::make_index_sequence<
						decltype(hana::size(maker_list))::value >()
				)
			, disposer_(disposer)
			, component_(component) {}

		/// \brief Get a reference to the disposer object
		::disposer::disposer& disposer(){ return disposer_; }


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "component(" << component_.name << ":"
				<< component_.type_name << "): ";
		}


	protected:
		::disposer::disposer& disposer_;

		component_base& component_;
	};


	/// \brief Wrapper for the component object creation function
	template < typename ComponentFn >
	class component_init{
	public:
		component_init(ComponentFn&& component_fn)
			: component_fn_(static_cast< ComponentFn&& >(component_fn)) {}

		template < typename Accessory >
		auto operator()(Accessory& accessory)const{
			if constexpr(std::is_invocable_v< ComponentFn const, Accessory& >){
				return component_fn_(accessory);
			}else if constexpr(std::is_invocable_v< ComponentFn const >){
				(void)accessory; // silance GCC
				return component_fn_();
			}else{
				static_assert(detail::false_c< Accessory >,
					"component_init function must be invokable with component& "
					"or without an argument");
			}
		}

	private:
		ComponentFn component_fn_;
	};


	/// \brief Maker function for \ref component in a std::unique_ptr
	template < typename MakerList, typename MakeData, typename ComponentFn >
	auto make_component_ptr(
		disposer& disposer,
		MakerList const& maker_list,
		MakeData const& data,
		std::string_view location,
		component_init< ComponentFn > const& component_fn
	){
		auto type = hana::unpack(maker_list, [](auto const& ... maker){
			return hana::type_c< hana::tuple<
				typename decltype(hana::typeid_(maker))::type::type ... > >;
		});

		using list_type = typename decltype(type)::type;

		return std::make_unique< component< list_type, ComponentFn > >(
			disposer, maker_list, data, location, component_fn);

	}


	/// \brief Provids types for constructing an component
	template <
		typename MakerList,
		typename ComponentFn,
		typename ComponentModules >
	struct component_maker{
		/// \brief Tuple of parameter-maker objects
		MakerList makers;

		/// \brief The function object that is called at construction
		component_init< ComponentFn > component_fn;

		/// \brief hana::tuple of \ref component_module_maker
		ComponentModules component_modules;


		/// \brief Create an component object
		auto operator()(
			component_make_data const& data,
			disposer& disposer
		)const{
			// Check config file data for undefined parameters and warn about
			// them
			auto const location = data.location();
			validate_iop< parameter_maker_tag >(
				location, makers, data.parameters);

			std::string const basic_location = data.basic_location();

			// Create the component
			auto result = make_component_ptr(
				disposer, makers, data, basic_location, component_fn);

			auto& component = *result.get();

			// register the component modules for this component instance
			hana::for_each(component_modules,
				[&data, &component, &disposer]
				(auto const& component_module_maker){
					component_module_maker.module_register_fn(component)(
						data.name + "//"
							+ std::string(component_module_maker.name),
						disposer.module_declarant()
					);
				});

			return result;
		}
	};


	/// \brief Wraps all given parameter configurations into a hana::tuple
	template < typename ... MakerList >
	constexpr auto component_configure(MakerList&& ... list){
		static_assert(hana::and_(hana::true_c,
			hana::is_a< parameter_maker_tag, MakerList >() ...),
			"at least one of the configure arguments is not a disposer "
			"parameter maker");

		return hana::make_tuple(static_cast< MakerList&& >(list) ...);
	}


	template < typename ... ComponentModules >
	constexpr auto component_modules(ComponentModules&& ... component_modules){
		static_assert(hana::and_(hana::true_c,
			hana::is_a< component_module_maker_tag, ComponentModules >() ...),
			"at least one of the configure arguments is not a disposer "
			"module maker");

		return hana::make_tuple(
			static_cast< ComponentModules&& >(component_modules) ...);
	}


}


#endif
