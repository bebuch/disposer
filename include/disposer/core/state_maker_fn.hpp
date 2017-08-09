//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__state_maker_fn__hpp_INCLUDED_
#define _disposer__core__state_maker_fn__hpp_INCLUDED_

#include "../tool/add_log.hpp"


namespace disposer{


	/// \brief Accessory of a module during enable/disable calls
	template < typename List, typename StateMakerFn, typename ExecFn >
	class state_accessory
		: public add_log< state_accessory< List, StateMakerFn, ExecFn > >{
	public:
		/// \brief Constructor
		state_accessory(module< List, StateMakerFn, ExecFn > const& module)
			: module_(module) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return module_.data()(name);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "chain(" << module_.chain() << ") module(" << module_.number()
				<< ":" << module_.type_name() << "): ";
		}


	private:
		/// \brief Reference to the module object
		module< List, StateMakerFn, ExecFn > const& module_;
	};


	/// \brief Wrapper for the module enable function
	template < typename StateMakerFn >
	class state_maker_fn{
	public:
		state_maker_fn(StateMakerFn&& maker_fn)
			: maker_fn_(static_cast< StateMakerFn&& >(maker_fn)) {}

		template < typename List, typename ExecFn >
		auto operator()(
			state_accessory< List, StateMakerFn, ExecFn > const& accessory
		)const{
			if constexpr(std::is_invocable_v< StateMakerFn const,
				state_accessory< List, StateMakerFn, ExecFn > const& >
			){
				static_assert(!std::is_void_v< std::invoke_result_t<
					StateMakerFn const,
					state_accessory< List, StateMakerFn, ExecFn > const& > >,
					"StateMakerFn must not return void");
				return maker_fn_(accessory);
			}else if constexpr(std::is_invocable_v< StateMakerFn const >){
				static_assert(!std::is_void_v< std::invoke_result_t<
					StateMakerFn const > >,
					"StateMakerFn must not return void");
				(void)accessory; // silance GCC
				return maker_fn_();
			}else{
				static_assert(detail::false_c< StateMakerFn >,
					"StateMakerFn function must be invokable with "
					"state_accessory const& or without an argument");
			}
		}

	private:
		StateMakerFn maker_fn_;
	};

	/// \brief Maker specialization for stateless modules
	template <> class state_maker_fn< void >{};


	/// \brief Holds the user defined state object of a module
	template < typename List, typename StateMakerFn, typename ExecFn >
	class state{
	public:
		/// \brief Constructor
		state(state_maker_fn< StateMakerFn > const& state_maker_fn)noexcept
			: state_maker_fn_(state_maker_fn) {}

		/// \brief Enables the module for exec calls
		///
		/// Build a users state object.
		void enable(module< List, StateMakerFn, ExecFn > const& module){
			state_.emplace(state_maker_fn_(state_accessory_type(module)));
		}

		/// \brief Disables the module for exec calls
		void disable()noexcept{
			state_.reset();
		}


	private:
		/// \brief Type for state_maker_fn
		using state_accessory_type =
			state_accessory< List, StateMakerFn, ExecFn >;

		/// \brief Type of the module state object
		using state_type = std::invoke_result_t<
			state_maker_fn< StateMakerFn >,
			state_accessory_type&& >;

		static_assert(!std::is_void_v< state_type >,
			"state_maker function must not return void");


		/// \brief The function object that is called in enable()
		state_maker_fn< StateMakerFn > state_maker_fn_;

		/// \brief The function object that is called in exec()
		std::optional< state_type > state_;
	};


	/// \brief Specialization for stateless modules
	template < typename List, typename ExecFn >
	class state< List, void, ExecFn >{
	public:
		/// \brief Constructor
		state(state_maker_fn< void > const&)noexcept{}

		/// \brief Module is stateless, do nothing
		void enable(module< List, void, ExecFn > const&)noexcept{}

		/// \brief Module is stateless, do nothing
		void disable()noexcept{}
	};


}


#endif
