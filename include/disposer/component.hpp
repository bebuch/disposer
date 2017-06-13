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
#include "add_log.hpp"

#include <type_traits>
#include <atomic>


namespace disposer{


	/// \brief Accessory of a \ref component without log
	template < typename Parameters >
	class component_config{
	public:
		/// \brief Constructor
		component_config(Parameters&& parameters)
			: parameters_(std::move(parameters)) {}


		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&& list)noexcept{
			return get(*this, static_cast< ConfigList&& >(list));
		}

		/// \brief Get reference to an parameter-object via
		///        its corresponding compile time name
		template < typename ConfigList >
		decltype(auto) operator()(ConfigList&& list)const noexcept{
			return get(*this, static_cast< ConfigList&& >(list));
		}


	private:
		/// \brief Implementation for \ref operator()
		template < typename Config, typename ConfigList >
		static decltype(auto) get(Config& config, ConfigList&&)noexcept{
			using io_t =
				std::remove_cv_t< std::remove_reference_t< ConfigList > >;
			static_assert(
				hana::is_a< parameter_name_tag, io_t >,
				"parameter io must be a parameter_name");


			static_assert(have_parameter(io_t::value),
				"parameter doesn't exist");
			return config.parameters_[io_t::value];
		}


		/// \brief hana::true_ if parameter_name exists
		template < typename String >
		constexpr static auto have_parameter(String){
			return
				hana::contains(decltype(hana::keys(parameters_))(), String());
		}

		/// \brief hana::map from parameter_name's to parameter's
		Parameters parameters_;
	};


	/// \brief Maker of \ref component_config
	template < typename Parameters >
	auto make_component_config(Parameters&& parameters){
		return component_config<
				std::remove_const_t< std::remove_reference_t< Parameters > >
			>(
				static_cast< Parameters&& >(parameters)
			);
	}


	/// \brief Accessory of a component
	template < typename Parameters >
	class component_accessory
		: public component_config< Parameters >
		, public add_log< component_accessory< Parameters > >
	{
	public:
		/// \brief Constructor
		component_accessory(
			component_base& component,
			Parameters&& parameters
		)
			: component_config< Parameters >(
					std::move(parameters)
				)
			, component_(component) {}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "component(" << component_.name << ":"
				<< component_.type_name << "): ";
		}


	protected:
		component_base& component_;
	};


	/// \brief The actual component type
	template <
		typename Parameters,
		typename ComponentFn >
	class component: public component_base{
	public:
		/// \brief Type for exec_fn
		using accessory_type = component_accessory< Parameters >;


// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		static_assert(std::is_callable_v<
			ComponentFn(accessory_type const&) >);

		using component_t = std::result_of_t<
			ComponentFn(accessory_type const&) >;
#else
		static_assert(std::is_invocable_v<
			ComponentFn, accessory_type const& >);

		using component_t = std::invoke_result_t<
			ComponentFn, accessory_type const& >;
#endif

		static_assert(!std::is_same_v< component_t, void >,
			"ComponentFn must return an object");


		/// \brief Constructor
		component(
			std::string const& name,
			std::string const& type_name,
			Parameters&& parameters,
			ComponentFn const& component_fn
		)
			: component_base(name, type_name)
			, accessory_(*this, std::move(parameters))
			, component_(component_fn(accessory_)) {}


	private:
		/// \brief Module access object for component_
		accessory_type accessory_;

		/// \brief The component object
		component_t component_;
	};


	/// \brief Maker function for \ref component in a std::unique_ptr
	template <
		typename Parameters,
		typename ComponentFn >
	auto make_component_ptr(
		std::string const& name,
		std::string const& type_name,
		Parameters&& parameters,
		ComponentFn&& component_fn
	){
		return std::make_unique< component<
				std::remove_const_t< std::remove_reference_t< Parameters > >,
				std::remove_const_t< std::remove_reference_t< ComponentFn > >
			> >(
				name, type_name,
				static_cast< Parameters&& >(parameters),
				static_cast< ComponentFn&& >(component_fn)
			);
	}


	/// \brief Provids types for constructing an component
	template <
		typename P_MakerList,
		typename ComponentFn >
	struct component_maker{
		/// \brief Tuple of parameter-maker objects
		P_MakerList makers;

		/// \brief The function object that is called at construction
		ComponentFn component_fn;


		/// \brief Create an component object
		auto operator()(component_make_data const& data)const{
			// Check config file data for undefined parameters and warn about
			// them
			auto const location = data.location();
			check_parameters(location, makers, data.parameters);

			std::string const basic_location = data.basic_location();

			// create parameter in the order of there definition in the
			// component
			auto p_list = hana::fold_left(makers, hana::make_tuple(),
				[&data, &location, &basic_location](auto&& get, auto&& maker){
					auto is_parameter =
						hana::is_a< parameter_maker_tag >(maker);

					static_assert(is_parameter,
						"maker is not a parameter (this is a bug in "
						"disposer!)");

					return hana::append(
						static_cast< decltype(get)&& >(get),
						maker(make_iop_list(iop_log{basic_location,
							"parameter", maker.name.c_str()}, get),
							make_parameter(location, maker,
								data.parameters)));
				}
			);

			// Create the component
			return make_component_ptr(
					data.name, data.type_name,
					as_iop_map(hana::filter(
						std::move(p_list), hana::is_a< parameter_tag >)),
					component_fn
				);
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

	struct unit_test_key;

	/// \brief Registers a component configuration in the \ref disposer
	template <
		typename P_MakerList,
		typename ComponentFn >
	class component_register_fn{
	public:
		/// \brief Constructor
		template <
			typename P_MakerListParam,
			typename ComponentFnParam >
		constexpr component_register_fn(
			P_MakerListParam&& list,
			ComponentFnParam&& component_fn
		)
			: called_flag_(false)
			, maker_{
				static_cast< P_MakerListParam&& >(list),
				static_cast< ComponentFnParam&& >(component_fn)
			}
			{}

		/// \brief Call this function to register the component with the given type
		///        name via the given component_declarant
		void operator()(std::string const& component_type, component_declarant& add){
			if(!called_flag_.exchange(true)){
				add(component_type,
					[maker{std::move(maker_)}](component_make_data const& data){
						return maker(data);
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
		component_maker< P_MakerList, ComponentFn > maker_;

		friend struct unit_test_key;
	};


	/// \brief Maker function for \ref component_register_fn
	template <
		typename IOP_MakerList,
		typename ComponentFn >
	constexpr auto make_component_register_fn(
		IOP_MakerList&& list,
		ComponentFn&& component_fn
	){
		return component_register_fn<
				std::remove_reference_t< IOP_MakerList >,
				std::remove_reference_t< ComponentFn >
			>(
				static_cast< IOP_MakerList&& >(list),
				static_cast< ComponentFn&& >(component_fn)
			);
	}


}


#endif
