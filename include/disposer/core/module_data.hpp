//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_data__hpp_INCLUDED_
#define _disposer__core__module_data__hpp_INCLUDED_

#include "ref.hpp"
#include "input.hpp"
#include "output.hpp"
#include "parameter.hpp"

#include <boost/hana/tuple.hpp>


namespace disposer{


	struct input_name_tag;
	struct output_name_tag;
	struct parameter_name_tag;


	/// \brief Ref of a \ref module without log
	template <
		typename TypeList,
		typename Inputs,
		typename Outputs,
		typename Parameters >
	struct module_data{
		/// \brief Constructor
		template < typename ... RefList >
		module_data(hana::tuple< RefList ... >&& ref_list)
			: inputs(hana::filter(std::move(ref_list),
				hana::is_a< input_tag >))
			, outputs(hana::filter(std::move(ref_list),
				hana::is_a< output_tag >))
			, parameters(hana::filter(std::move(ref_list),
				hana::is_a< parameter_tag >)) {}

		/// \brief hana::tuple of the parameters
		Inputs inputs;

		/// \brief hana::tuple of the outputs
		Outputs outputs;

		/// \brief hana::tuple of the parameters
		Parameters parameters;
	};


}


#endif
