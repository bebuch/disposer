//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__parameter__hpp_INCLUDED_
#define _disposer__core__parameter__hpp_INCLUDED_

#include "parameter_name.hpp"

#include "../tool/to_std_string.hpp"


namespace disposer{


	/// \brief Hana Tag for parameter
	struct parameter_tag{};

	/// \brief The parameter type
	template < typename Name, typename T, bool IsRequired >
	class parameter{
	public:
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		static_assert(hana::is_a< parameter_name_tag, Name >);
#endif

		/// \brief Hana tag to identify parameters
		using hana_tag = parameter_tag;


		/// \brief Whether the parameter is always available
		static constexpr auto is_required = std::bool_constant< IsRequired >();


		/// \brief Compile time name of the parameter
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		parameter(std::unique_ptr< T const >&& value):
			value_(std::move(value)){
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
				if constexpr(is_required) assert(value_);
#endif
			}

		/// \brief Does the parameter have a value
		bool is_available()noexcept{
			if constexpr(is_required){
				static_assert(false_c< IsRequired >,
					"Parameter is required and therefore always available! "
					"Just don't ask ;-)");
			}

			return value_;
		}

		/// \brief Access the value if parameter has only one type
		T const& get()const noexcept(is_required){
			if constexpr(!is_required){
				if(!value_) throw std::logic_error("parameter("
					+ to_std_string(name) + ") has no value");
			}

			return *value_;
		}


	private:
		/// \brief The value
		std::unique_ptr< T const > value_;
	};


}


#endif
