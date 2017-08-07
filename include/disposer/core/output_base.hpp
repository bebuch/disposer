//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_base__hpp_INCLUDED_
#define _disposer__core__output_base__hpp_INCLUDED_

#include "../tool/type_index.hpp"

#include <string_view>
#include <map>


namespace disposer{



	/// \brief Base for module outpus
	///
	/// Polymorphe base class for module outputs.
	///
	/// A disposer module output must have at least one output data type.
	/// An output might have more then one data type.
	class output_base{
	public:
		/// \brief Outputs are not copyable
		output_base(output_base const&) = delete;

		/// \brief Outputs are not movable
		output_base(output_base&&) = delete;


		/// \brief Outputs are not copy-assignable
		output_base& operator=(output_base const&) = delete;

		/// \brief Outputs are not move-assignable
		output_base& operator=(output_base&&) = delete;


		/// \brief List of enabled output types
		virtual std::map< type_index, bool > enabled_types()const = 0;
	};


}


#endif
