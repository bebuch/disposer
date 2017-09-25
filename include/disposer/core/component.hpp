//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component__hpp_INCLUDED_
#define _disposer__core__component__hpp_INCLUDED_

#include "component_base.hpp"
#include "component_init_fn_fn.hpp"
#include "module_name.hpp"

#include "../config/component_make_data.hpp"
#include "../config/validate_iop.hpp"


namespace disposer{


	/// \brief The actual component type
	template < typename List, typename ComponentInitFn >
	class component: public component_base{
	public:
		/// \brief Type for exec_fn
		using accessory_type = component_accessory< List >;


		using component_t = std::invoke_result_t<
			component_init_fn< ComponentInitFn >, accessory_type& >;

		static_assert(!std::is_same_v< component_t, void >,
			"ComponentInitFn must return an object");


		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		component(
			disposer& disposer,
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			component_init_fn< ComponentInitFn > const& component_fn
		)
			: component_base(data.name, data.type_name)
			, accessory_(*this, disposer, maker_list, data, location)
			, component_(component_fn(accessory_)) {}


		/// \brief Components are not copyable
		component(component const&) = delete;

		/// \brief Components are not movable
		component(component&&) = delete;


		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		auto& operator()(P const& param)noexcept{
			return accessory_(param);
		}

		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		auto const& operator()(P const& param)const noexcept{
			return accessory_(param);
		}


		/// \brief Access to the component object
		component_t& data()noexcept{ return component_; }

		/// \brief Access to the const component object
		component_t const& data()const noexcept{ return component_; }


	private:
		/// \brief shutdown component if it has a shutdown function
		///
		/// Every component which controls the disposer asynchronous should
		/// implement such a function.
		void shutdown()override{
			auto has_shutdown =
				hana::is_valid([](auto& t)->decltype((void)t.shutdown()){})
				(component_);
			if constexpr(has_shutdown){
				component_.shutdown();
			}
		}


		/// \brief Module access object for component_
		accessory_type accessory_;

		/// \brief The component object
		component_t component_;
	};


	/// \brief Maker function for \ref component in a std::unique_ptr
	template < typename MakerList, typename MakeData, typename ComponentInitFn >
	auto make_component_ptr(
		disposer& disposer,
		MakerList const& maker_list,
		MakeData const& data,
		std::string_view location,
		component_init_fn< ComponentInitFn > const& component_fn
	){
		auto type = hana::unpack(maker_list, [](auto const& ... maker){
			return hana::type_c< hana::tuple<
				typename decltype(hana::typeid_(maker))::type::type ... > >;
		});

		using list_type = typename decltype(type)::type;

		return std::make_unique< component< list_type, ComponentInitFn > >(
			disposer, maker_list, data, location, component_fn);

	}


	/// \brief Provids types for constructing an component
	template <
		typename MakerList,
		typename ComponentInitFn,
		typename ComponentModules >
	struct component_maker{
		/// \brief Tuple of parameter-maker objects
		MakerList makers;

		/// \brief The function object that is called at construction
		component_init_fn< ComponentInitFn > component_fn;

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


	/// \brief Wraps all given P configurations into a hana::tuple
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
