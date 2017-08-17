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


	template < typename StateType, typename List >
	class exec_accessory;

	/// \brief Class exec_input_key access key
	struct exec_key{
	private:
		/// \brief Constructor
		constexpr exec_key()noexcept = default;

		template < typename StateType, typename List >
		friend class exec_accessory;
	};


	/// \brief Accessory of a module during exec calls
	template < typename StateType, typename List >
	class exec_accessory: public add_log< exec_accessory< StateType, List > >{
	public:
		/// \brief Constructor
		exec_accessory(
			std::size_t id,
			module_data< List > const& data,
			exec_module_data< detail::exec_list_t< List > >& exec_data,
			StateType* state,
			std::string_view location
		)
			: id_(id)
			, data_(data)
			, exec_data_(exec_data)
			, state_(state)
			, location_(location) {}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return get(*this, name);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return get(*this, name);
		}

		/// \brief Get access to the state object if one exists
		auto& state()noexcept{
			static_assert(!std::is_void_v< StateType >,
				"Module has no state.");
			return *state_;
		}

		/// \brief ID of this exec
		std::size_t id()noexcept{ return id_; }


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location_;
		}


	private:
		template < typename This, typename Name >
		static auto& get(This& this_, Name const& name)noexcept{
			using name_t = std::remove_reference_t< Name >;
			if constexpr(hana::is_a< parameter_name_tag, name_t >()){
				return this_.data_(name);
			}else if constexpr(hana::is_a< input_name_tag, name_t >()){
					return this_.exec_data_(name);
			}else if constexpr(hana::is_a< output_name_tag, name_t >()){
				// TODO: Must be tested!!!
// 				if(!this_.data_(name).is_enabled[hana::type_c< V >]){
// 					using namespace std::literals::string_literals;
// 					throw std::logic_error(io_tools::make_string(
// 						"output '", detail::to_std_string_view(name),
// 						"' with disabled type [", type_name< V >(),
// 						"] requested"
// 					));
// 				}

				return this_.exec_data_(name);
			}else{
				static_assert(detail::false_c< Name >,
					"name is not an input_name, output_name or parameter_name");
			}
		}


		/// \brief Current exec id
		std::size_t id_;

		/// \brief Data of the module
		module_data< List > const& data_;

		/// \brief Data of the exec_module
		exec_module_data< detail::exec_list_t< List > >& exec_data_;

		/// \brief Module state
		///
		/// nullptr-pointer to void if module is stateless.
		StateType* state_;

		/// \brief Prefix for log messages
		std::string_view location_;
	};


	/// \brief Wrapper for the module exec function
	template < typename Fn >
	class exec_fn{
	public:
		constexpr exec_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit exec_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit exec_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(static_cast< Fn&& >(fn)) {}


		template < typename StateType, typename List >
		void operator()(exec_accessory< StateType, List >& accessory){
			if constexpr(std::is_invocable_v< Fn,
				exec_accessory< StateType, List >& >
			){
				fn_(accessory);
			}else if constexpr(std::is_invocable_v< Fn >){
				(void)accessory; // silance GCC
				fn_();
			}else{
				static_assert(detail::false_c< StateType >,
					"Fn must be invokable with exec_accessory& or "
					"without arguments");
			}
		}


	private:
		Fn fn_;
	};


}


#endif
