//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component_init_fn__hpp_INCLUDED_
#define _disposer__core__component_init_fn__hpp_INCLUDED_

#include "component_data.hpp"


namespace disposer{


	/// \brief Accessory of a component
	template < typename TypeList, typename Parameters >
	class component_init_accessory
		: public add_log< component_init_accessory< TypeList, Parameters > >{
	public:
		/// \brief Constructor
		component_init_accessory(
			component_data< TypeList, Parameters > const& data,
			std::string_view location
		)
			: data_(data)
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


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location_;
		}


	private:
		/// \brief Reference to the component object
		component_data< TypeList, Parameters > const& data_;

		/// \brief Location for log messages
		std::string_view location_;
	};


	/// \brief Wrapper for the component enable function
	template < typename Fn = void >
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
			: fn_(static_cast< Fn&& >(fn)) {}

		template < typename TypeList, typename Parameters >
		auto operator()(
			component_init_accessory< TypeList, Parameters > const& accessory
		)const{
			// TODO: calulate noexcept
			if constexpr(std::is_invocable_v< Fn const,
				component_init_accessory< TypeList, Parameters > const& >
			){
				static_assert(!std::is_void_v< std::invoke_result_t<
					Fn const, component_init_accessory< TypeList,
						Parameters > const& > >,
					"Fn must not return void");
				return std::invoke(fn_, accessory);
			}else if constexpr(std::is_invocable_v< Fn const >){
				static_assert(!std::is_void_v< std::invoke_result_t<
					Fn const > >,
					"Fn must not return void");
				(void)accessory; // silance GCC
				return std::invoke(fn_);
			}else{
				static_assert(detail::false_c< Fn >,
					"Fn function must be invokable with "
					"component_init_accessory const& or without an argument");
			}
		}

	private:
		Fn fn_;
	};


	/// \brief Holds the user defined state object of a component
	template <
		typename TypeList,
		typename Parameters,
		typename ComponentInitFn >
	class component_state{
	public:
		/// \brief Type of the component state object
		using state_type = std::invoke_result_t<
			component_init_fn< ComponentInitFn >,
			component_init_accessory< TypeList, Parameters >&& >;


		/// \brief Constructor
		component_state(
			component_init_fn< ComponentInitFn > const& component_init_fn
		)noexcept
			: state_(component_init_fn_(component_init_accessory
				< TypeList, Parameters >(data, location))) {}



	private:
		/// \brief The function object that is called in exec()
		state_type state_;
	};


}


#endif
