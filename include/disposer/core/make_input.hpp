//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_input__hpp_INCLUDED_
#define _disposer__core__make_input__hpp_INCLUDED_

#include "input_name.hpp"
#include "dimension_solve.hpp"
#include "output_base.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/to_std_string_view.hpp"

#include <array>
#include <variant>
#include <unordered_map>


namespace disposer{


	using namespace std::literals::string_view_literals;


	/// \brief Provids types for constructing an input
	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;
	};


	/// \brief Configuration class for inputs
	template < bool IsRequired >
	struct is_required: std::bool_constant< IsRequired >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto required = is_required< true >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto not_required = is_required< false >{};


	/// \brief Creates a \ref input_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		bool IsRequired = true >
	constexpr auto make(
		input_name< C ... > const&,
		dimension_referrer< Template, D ... > const&,
		is_required< IsRequired > = required
	){
		return input_maker<
			input_name< C ... >,
			dimension_referrer< Template, D ... >,
			IsRequired >{};
	}


	inline output_base* get_output_ptr(
		input_list const& inputs,
		std::string_view const& name
	){
		auto const iter = inputs.find(name);
		return iter != inputs.end() ? iter->second : nullptr;
	}


	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto deduce_dimensions_by_input(
		input_maker< Name, DimensionReferrer, IsRequired >,
		dimension_list< Ds ... >,
		output_base* const output_ptr
	){
		if constexpr(IsRequired){
			constexpr dimension_solver solver(
				dimension_list< Ds ... >{},
				DimensionReferrer{});
			return solver.solve(Name{}, output_ptr->get_type());
		}else{
			(void)output_ptr; // Silance GCC
			return solved_dimensions{};
		}
	}


}


#endif
