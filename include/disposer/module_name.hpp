//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_name__hpp_INCLUDED_
#define _disposer__module_name__hpp_INCLUDED_

#include "ct_name.hpp"
#include "config_fn.hpp"
#include "module.hpp"

#include <string_view>


namespace disposer{


	/// \brief Hana Tag for \ref module_name
	struct component_module_maker_tag{};

	/// \brief Data to create a component module
	template < typename ModuleRegisterFn >
	struct component_module_maker{
		/// \brief Hana tag to identify component module makers
		using hana_tag = component_module_maker_tag;

		/// \brief Name of the component module
		std::string_view name;

		/// \brief A list of module IOPs
		ModuleRegisterFn module_register_fn;
	};


	/// \brief Hana Tag for \ref module_name
	struct module_name_tag{};

	/// \brief A compile time string type for modules
	template < char ... C >
	struct module_name: ct_name< C ... >{
		/// \brief Hana tag to identify module names
		using hana_tag = module_name_tag;

		/// \brief Creates a \ref module_register_fn object
		template < typename ModuleRegisterFn >
		constexpr auto operator()(
			ModuleRegisterFn&& module_register_fn
		)const{
			return component_module_maker<
				std::remove_reference_t< ModuleRegisterFn >
			>{
				to_std_string_view(this->value),
				static_cast< ModuleRegisterFn&& >(module_register_fn)
			};
		}
	};

	/// \brief Make a \ref module_name object
	template < char ... C >
	module_name< C ... > module_name_c{};


	/// \brief Make a \ref module_name object by a hana::string object
	template < char ... C > constexpr module_name< C ... >
	to_module_name(hana::string< C ... >)noexcept{ return {}; }


}

#define DISPOSER_MODULE(s) \
	::disposer::to_module_name(BOOST_HANA_STRING(s))


#ifdef BOOST_HANA_CONFIG_ENABLE_STRING_UDL
namespace disposer::literals{


	/// \brief Make a \ref module_name object via a user defined literal
	template < typename CharT, CharT ... c >
	constexpr auto operator"" _module(){
		static_assert(std::is_same_v< CharT, char >);
		return module_name_c< c ... >;
	}


}
#endif


#endif
