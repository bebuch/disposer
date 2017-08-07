//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_fn__hpp_INCLUDED_
#define _disposer__core__exec_fn__hpp_INCLUDED_

#include "../tool/add_log.hpp"


namespace disposer{


	template < typename Module >
	class exec_accessory;

	/// \brief Class input_exec_key access key
	struct exec_key{
	private:
		/// \brief Constructor
		constexpr exec_key()noexcept = default;

		template < typename Module >
		friend class exec_accessory;
	};


	/// \brief Accessory of a module during exec calls
	template < typename Module >
	class exec_accessory: public add_log< exec_accessory< Module > >{
	public:
		/// \brief Constructor
		exec_accessory(Module& module)
			: module_(module) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		auto const& operator()(IOP const& iop)noexcept const{
			return module_.data()(iop);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		auto& operator()(IOP const& iop)noexcept{
			return module_.data()(iop);
		}

		/// \brief Get access to the state object if one exists
		auto& state()noexcept{
			static_assert(!std::is_void_v< typename Module::state_type >,
				"Module have no state.");
			return module_.state();
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << "id(" << this->module_.id << ") chain("
				<< this->module_.chain << ") module(" << this->module_.number
				<< ":" << this->module_.type_name << ") exec: ";
		}


	private:
		/// \brief Reference to the module object
		Module& module_;
	};


	/// \brief Wrapper for the module exec function
	template < typename ExecFn >
	class exec_fn{
	public:
		exec_fn(ExecFn&& exec_fn)
			: exec_fn_(static_cast< ExecFn&& >(exec_fn)) {}


		template < typename Module >
		void operator()(exec_accessory< Module >& accessory){
			if constexpr(std::is_invocable_v< ExecFn,
				exec_accessory< Module >& >
			){
				exec_fn_(accessory);
			}else if constexpr(std::is_invocable_v< ExecFn >){
				(void)accessory; // silance GCC
				exec_fn_();
			}else{
				static_assert(detail::false_c< Module >,
					"ExecFn must be invokable with exec_accessory& or "
					"without arguments");
			}
		}


	private:
		ExecFn exec_fn_;
	};


}


#endif
