//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_output__hpp_INCLUDED_
#define _disposer__core__make_output__hpp_INCLUDED_

#include "output_name.hpp"
#include "dimension_referrer.hpp"
#include "dimension_referrer_output.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/to_std_string_view.hpp"

#include <variant>
#include <unordered_map>


namespace disposer{


	/// \brief Provid types for constructing an output
	template <
		typename Name,
		typename DimensionReferrer >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;

		/// \brief output_name object
		static constexpr auto name = Name{};

		/// \brief Description of the output
		template < typename ... DTs >
		std::string help_text_fn(dimension_list< DTs ... >)const{
			std::ostringstream help;
			help << "    * output: "
				<< detail::to_std_string_view(name) << "\n";
			help << wrapped_type_ref_text(
				DimensionReferrer{}, dimension_list< DTs ... >{});
			return help.str();
		}
	};


	/// \brief Creates a \ref output_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D >
	auto make(
		output_name< C ... > const&,
		dimension_referrer< Template, D ... > const&
	){
		return output_maker<
			output_name< C ... >,
			dimension_referrer< Template, D ... > >{};
	}


	inline std::size_t get_use_count(
		output_list const& outputs,
		std::string_view const& name
	){
		auto const iter = outputs.find(name);
		return iter != outputs.end() ? iter->second : 0;
	}


}


#endif
