//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__component__hpp_INCLUDED_
#define _disposer__component__hpp_INCLUDED_

#include "parameter.hpp"
#include "component_base.hpp"
#include "component_make_data.hpp"
#include "module.hpp"
#include "module_name.hpp"
#include "add_log.hpp"

#include <type_traits>
#include <atomic>


namespace disposer{


	/// \brief Accessory of a \ref component without log
	template < typename IOP_List >
	class component_config{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData, std::size_t ... I >
		component_config(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			std::index_sequence< I ... >
		)
			: // iop_list_ can be referenced before initialization
				iop_ref_list_(hana::transform(iop_list_, ref{}))
			, iop_list_(iops_make_data(
				iop_make_data(maker_list[hana::size_c< I >], data, location),
				location, hana::slice_c< 0, I >(iop_ref_list_),
				hana::size_c< I >) ...)
		{
			(void)location; // GCC bug (silance unused warning)
		}


		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		decltype(auto) operator()(P const& param)noexcept{
			return get(*this, param);
		}

		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		decltype(auto) operator()(P const& param)const noexcept{
			return get(*this, param);
		}


	private:
		/// \brief Implementation for \ref operator()
		template < typename Config, typename IOP >
		static decltype(auto) get(Config& config, IOP const& iop)noexcept{
			using iop_t = std::remove_reference_t< IOP >;
			static_assert(
				hana::is_a< parameter_name_tag, iop_t >,
				"parameter is not an parameter_name");

			using iop_tag = typename iop_t::hana_tag;

			auto iop_ref = hana::find_if(config.iop_list_, [&iop](auto ref){
				using tag = typename decltype(ref)::type::name_type::hana_tag;
				return hana::type_c< iop_tag > == hana::type_c< tag >
					&& ref.get().name == iop.value;
			});

			auto is_iop_valid = iop_ref != hana::nothing;
			static_assert(is_iop_valid, "requested iop doesn't exist");

			return iop_ref->get();
		}


		/// \brief Like IOP_List but with elements in std::reference_wrapper
		using iop_ref_list_type =
			decltype(hana::transform(std::declval< IOP_List& >(), ref{}));

		/// \brief hana::tuple of references to inputs, outputs and parameters
		iop_ref_list_type iop_ref_list_;

		/// \brief hana::tuple of the inputs, outputs and parameters
		IOP_List iop_list_;
	};


	/// \brief Accessory of a component
	template < typename IOP_List >
	class component_accessory
		: public component_config< IOP_List >
		, public add_log< component_accessory< IOP_List > >
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
			: component_config< IOP_List >(
					maker_list, data, location, std::make_index_sequence<
						decltype(hana::size(maker_list))::value >()
				)
			, disposer_(disposer)
			, component_(component) {}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "component(" << component_.name << ":"
				<< component_.type_name << "): ";
		}

		/// \brief Get a reference to the disposer object
		::disposer::disposer& disposer(){ return disposer_; }


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
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			if constexpr(std::is_callable_v< ComponentFn const(Accessory&) >){
				return component_fn_(accessory);
			}else if constexpr(std::is_callable_v< ComponentFn const() >){
				return component_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"component_init function must be invokable with component& "
					"or without an argument");
			}
	#else
			if constexpr(std::is_invocable_v< ComponentFn const, Accessory& >){
				return component_fn_(accessory);
			}else if constexpr(std::is_invocable_v< ComponentFn const >){
				(void)accessory; // silance GCC
				return component_fn_();
			}else{
				static_assert(false_c< Accessory >,
					"component_init function must be invokable with component& "
					"or without an argument");
			}
#endif
		}

	private:
		ComponentFn const component_fn_;
	};


	/// \brief The actual component type
	template < typename IOP_List, typename ComponentFn >
	class component: public component_base{
	public:
		/// \brief Type for exec_fn
		using accessory_type = component_accessory< IOP_List >;


// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		using component_t = std::result_of_t<
			component_init< ComponentFn >(accessory_type&) >;
#else
		using component_t = std::invoke_result_t<
			component_init< ComponentFn >, accessory_type& >;
#endif

		static_assert(!std::is_same_v< component_t, void >,
			"ComponentFn must return an object");


		/// \brief Constructor
		template < typename MakerList, typename MakeData >
		component(
			disposer& disposer,
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			component_init< ComponentFn > const& component_fn
		)
			: component_base(data.name, data.type_name)
			, accessory_(*this, disposer, maker_list, data, location)
			, component_(component_fn(accessory_)) {}


		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		decltype(auto) operator()(P&& param)noexcept{
			return accessory_(static_cast< P&& >(param));
		}

		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename P >
		decltype(auto) operator()(P&& param)const noexcept{
			return accessory_(static_cast< P&& >(param));
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
		void shutdown(disposer_key const&)override{
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
	template < typename MakerList, typename MakeData, typename ComponentFn >
	auto make_component_ptr(
		disposer& disposer,
		MakerList const& maker_list,
		MakeData const& data,
		std::string_view location,
		component_init< ComponentFn > const& component_fn
	){
		auto list_type = hana::unpack(maker_list, [](auto const& ... maker){
			return hana::type_c< decltype(hana::make_tuple(
					std::declval< typename
						decltype(hana::typeid_(maker))::type::type >() ...
				)) >;
		});

		using iop_list_type = typename decltype(list_type)::type;

		return std::make_unique< component< iop_list_type, ComponentFn > >(
			disposer, maker_list, data, location, component_fn);

	}


	/// \brief Provids types for constructing an component
	template <
		typename P_MakerList,
		typename ComponentFn,
		typename ComponentModules >
	struct component_maker{
		/// \brief Tuple of parameter-maker objects
		P_MakerList makers;

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
			check_parameters(location, makers, data.parameters);

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
						data.name + "_"
							+ std::string(component_module_maker.name),
						disposer.module_declarant()
					);
				});

			return result;
		}
	};


	/// \brief Wraps all given P configurations into a hana::tuple
	template < typename ... P_MakerList >
	constexpr auto component_configure(P_MakerList&& ... list){
		static_assert(hana::and_(hana::true_c,
			hana::is_a< parameter_maker_tag, P_MakerList >() ...),
			"at least one of the configure arguments is not a disposer "
			"parameter maker");

		return hana::make_tuple(static_cast< P_MakerList&& >(list) ...);
	}


	template < typename ... ComponentModules >
	auto component_modules(ComponentModules&& ... component_modules){
		static_assert(hana::and_(hana::true_c,
			hana::is_a< component_module_maker_tag, ComponentModules >() ...),
			"at least one of the configure arguments is not a disposer "
			"module maker");

		return hana::make_tuple(
			static_cast< ComponentModules&& >(component_modules) ...);
	}


	struct unit_test_key;

	/// \brief Registers a component configuration in the \ref disposer
	template <
		typename P_MakerList,
		typename ComponentFn,
		typename ComponentModules >
	class component_register_fn{
	public:
		/// \brief Constructor
		constexpr component_register_fn(
			P_MakerList&& list,
			component_init< ComponentFn >&& component_fn,
			ComponentModules&& component_modules
		)
			: called_flag_(false)
			, maker_{
				static_cast< P_MakerList&& >(list),
				std::move(component_fn),
				static_cast< ComponentModules&& >(component_modules)
			}
			{}

		/// \brief Call this function to register the component with the given
		///        type name via the given component_declarant
		void operator()(
			std::string const& component_type,
			component_declarant& add
		){
			if(!called_flag_.exchange(true)){
				add(component_type,
					[maker{std::move(maker_)}, &add]
					(component_make_data const& data){
						return maker(data, add.disposer());
					});
			}else{
				throw std::runtime_error("called register function '"
					+ component_type + "' more than once");
			}

		}


	private:
		/// \brief Operator must only called once!
		std::atomic< bool > called_flag_;

		/// \brief The component_maker object
		component_maker< P_MakerList, ComponentFn, ComponentModules > maker_;

		friend struct unit_test_key;
	};


}


#endif
