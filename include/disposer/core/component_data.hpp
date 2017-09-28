//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__component_data__hpp_INCLUDED_
#define _disposer__core__component_data__hpp_INCLUDED_

#include "accessory.hpp"
#include "parameter.hpp"

#include <boost/hana/tuple.hpp>


namespace disposer{


	struct parameter_name_tag;


	/// \brief Accessory of a \ref component without log
	template < typename TypeList, typename Parameters >
	struct component_data{
		/// \brief Constructor
		template < typename ... RefList >
		component_data(hana::tuple< RefList ... >&& ref_list)
			: parameters(hana::filter(std::move(ref_list),
				hana::is_a< parameter_tag >)) {}

		/// \brief hana::tuple of the parameters
		Parameters parameters;
	};


}


#endif
