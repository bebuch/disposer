//-----------------------------------------------------------------------------
// Copyright (c) 2015-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_info__hpp_INCLUDED_
#define _disposer__core__exec_info__hpp_INCLUDED_

#include <cstddef>


namespace disposer{


	/// \brief Returned by an exec call
	struct exec_info{
		/// \brief true if exec was successfully executed, false otherwise
		bool success;

		/// \brief Disposer global execution ID
		std::size_t id;

		/// \brief Chain local execution ID
		std::size_t exec_id;


		/// \brief Implicit conversion to bool
		constexpr operator bool()const noexcept{
			return success;
		}
	};


}


#endif
