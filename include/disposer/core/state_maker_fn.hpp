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
	template < typename Module >
	class state_accessory: public add_log< state_accessory< Module > >{
	public:
		/// \brief Constructor
		state_accessory(Module const& module)
			: module_(module) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		auto const& operator()(IOP const& iop)noexcept const{
			return module_.data()(iop);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "chain(" << module_.chain << ") module(" << module_.number
				<< ":" << module_.type_name << "): ";
		}


	private:
		/// \brief Reference to the module object
		Module const& module_;
	};


	/// \brief Wrapper for the module enable function
	template < typename StateMakerFn >
	class state_maker_fn{
	public:
		state_maker_fn(StateMakerFn&& maker_fn)
			: maker_fn_(static_cast< StateMakerFn&& >(maker_fn)) {}

		template < typename Module >
		auto operator()(state_accessory< Module > const& accessory)const{
			if constexpr(std::is_invocable_v< StateMakerFn const,
				state_accessory< Module > const& >
			){
				static_assert(!std::is_void_v< std::invoke_result_t<
					StateMakerFn const, state_accessory< Module > const& > >,
					"StateMakerFn must not return void");
				return maker_fn_(accessory);
			}else if constexpr(std::is_invocable_v< StateMakerFn const >){
				static_assert(!std::is_void_v< std::invoke_result_t<
					StateMakerFn const > >,
					"StateMakerFn must not return void");
				(void)accessory; // silance GCC
				return maker_fn_();
			}else{
				static_assert(detail::false_c< Module >,
					"StateMakerFn function must be invokable with "
					"state_accessory const& or without an argument");
			}
		}

	private:
		StateMakerFn maker_fn_;
	};


}


#endif
