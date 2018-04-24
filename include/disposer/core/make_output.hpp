//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
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

#include <boost/algorithm/string/replace.hpp>

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

		/// \brief Dimension dependence of the type
		using dimension_referrer = DimensionReferrer;

		/// \brief output_name object
		static constexpr auto name = Name{};

		/// \brief True if is type is not dependet on anything
		static constexpr bool is_free_type = DimensionReferrer::is_free_type;


		/// \brief Description of the output
		template < typename ... DTs >
		std::string help_text_fn(dimension_list< DTs ... >)const{
			std::ostringstream help;
			help << "    * output: "
				<< detail::to_std_string_view(name) << "\n";
			help << help_text << "\n";
			help << wrapped_type_ref_text(
				DimensionReferrer{}, dimension_list< DTs ... >{});
			return help.str();
		}


		/// \brief User defined help text
		std::string const help_text;
	};


	/// \brief Creates a \ref output_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D >
	auto make(
		output_name< C ... > const&,
		dimension_referrer< Template, D ... > const&,
		std::string const& description
	){
		return output_maker<
			output_name< C ... >,
			dimension_referrer< Template, D ... > >{
				"      * " +
				boost::replace_all_copy(description, "\n", "\n        ")
			};
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
