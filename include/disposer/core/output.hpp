//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output__hpp_INCLUDED_
#define _disposer__core__output__hpp_INCLUDED_

#include "output_base.hpp"
#include "output_name.hpp"

#include <boost/hana/core/is_a.hpp>


namespace disposer{


	/// \brief Hana Tag for output
	struct output_tag{};

	/// \brief The actual output type
	template < typename Name, typename T >
	class output: public output_base{
	public:
		static_assert(hana::is_a< output_name_tag, Name >);


		/// \brief Hana tag to identify outputs
		using hana_tag = output_tag;


		/// \brief Compile time name of the output
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		output(std::size_t use_count)
			: output_base(use_count) {}


	private:
		/// \brief Type index of the output type
		virtual type_index get_type()const override{
			return type_index::type_id< T >();
		}
	};


}


#endif
