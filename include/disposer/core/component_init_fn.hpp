//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component_init_fn__hpp_INCLUDED_
#define _disposer__core__component_init_fn__hpp_INCLUDED_

#include "component_base.hpp"
#include "component_data.hpp"
#include "system.hpp"


namespace disposer{


	/// \brief Same interface as system, but without load_config and with a
	///        check that the component does not remove itself
	class component_system_proxy{
	public:
		component_system_proxy(
			system& system,
			std::string_view component_name
		)
			: system_(system)
			, component_name_(component_name) {}


		/// \brief Remove a component
		void remove_component(std::string const& name){
			if(name == component_name_){
				throw std::logic_error("component(" + name
					+ ") tried to remove itself");
			}
			system_.remove_component(name);
		}

		/// \brief Create a component
		void load_component(std::istream& content){
			system_.load_component(content);
		}

		/// \brief Remove a chain
		void remove_chain(std::string const& name){
			system_.remove_chain(name);
		}

		/// \brief Create a chain
		void load_chain(std::istream& content){
			system_.load_chain(content);
		}


		/// \brief The directory object
		disposer::directory& directory(){
			return system_.directory();
		}

		/// \brief The directory object
		disposer::directory const& directory()const{
			return system_.directory();
		}


		/// \brief List of all chaines
		std::unordered_set< std::string > chains()const{
			return system_.chains();
		}


		/// \brief Get a reference to the chain, throw if it does not exist
		enabled_chain enable_chain(std::string const& chain){
			return system_.enable_chain(chain);
		}


	private:
		/// \brief Reference to the actual system
		system& system_;

		/// \brief Name of the component
		std::string_view component_name_;
	};


	/// \brief Ref of a component
	template < typename TypeList, typename Parameters >
	class component_init_ref
		: public add_log< component_init_ref< TypeList, Parameters > >{
	public:
		/// \brief Constructor
		component_init_ref(
			component_data< TypeList, Parameters > const& data,
			std::string_view component_name,
			disposer::system& system,
			std::string_view location
		)
			: data_(data)
			, system_(system, component_name)
			, location_(location) {}


		/// \brief Get reference to a parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		decltype(auto) operator()(Name const& name)const noexcept{
			using name_t = std::remove_reference_t< Name >;
			if constexpr(hana::is_a< parameter_name_tag, name_t >()){
				auto const index = hana::index_if(data_.parameters,
					[name](auto const& p){ return p.name == name; });

				static_assert(!hana::is_nothing(index),
					"parameter name doesn't exist");

				return data_.parameters[*index].get();
			}else{
				static_assert(detail::false_c< Name >,
					"name is not an parameter_name");
			}
		}

		/// \brief Get type by dimension index
		template < std::size_t DI >
		static constexpr auto dimension(hana::size_t< DI > i)noexcept{
			static_assert(DI < TypeList::type_count,
				"component has less then DI dimensions");
			return TypeList::types[i];
		}

		/// \brief Get reference to the system object
		component_system_proxy& system()noexcept{
			return system_;
		}

		/// \brief Get const reference to the system object
		component_system_proxy const& system()const noexcept{
			return system_;
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location_;
		}


	private:
		/// \brief Reference to the component object
		component_data< TypeList, Parameters > const& data_;

		/// \brief Reference to the system object
		component_system_proxy system_;

		/// \brief Location for log messages
		std::string_view location_;
	};


	/// \brief Wrapper for the component enable function
	template < typename Fn >
	class component_init_fn{
	public:
		constexpr component_init_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

 		constexpr explicit component_init_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

 		constexpr explicit component_init_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}

		template < typename TypeList, typename Parameters >
		auto operator()(
			component_init_ref< TypeList, Parameters >&& ref
		)const{
			// TODO: calulate noexcept
			if constexpr(std::is_invocable_v< Fn const,
				component_init_ref< TypeList, Parameters > >
			){
				static_assert(!std::is_void_v< std::invoke_result_t<
					Fn const, component_init_ref< TypeList,
						Parameters > > >,
					"Fn must not return void");
				return std::invoke(fn_, std::move(ref));
			}else if constexpr(std::is_invocable_v< Fn const >){
				static_assert(!std::is_void_v< std::invoke_result_t<
					Fn const > >,
					"Fn must not return void");
				(void)ref; // silance GCC
				return std::invoke(fn_);
			}else{
				static_assert(detail::false_c< Fn >,
					"Fn function must be const invokable with "
					"component_init_ref or without an argument");
			}
		}

	private:
		Fn fn_;
	};


}


#endif
