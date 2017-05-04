//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__iop_list__hpp_INCLUDED_
#define _disposer__iop_list__hpp_INCLUDED_

#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"


namespace disposer{


	/// \brief Tag for input_maker
	struct input_maker_tag{};

	/// \brief Tag for output_maker
	struct output_maker_tag{};

	/// \brief Tag for parameter_maker
	struct parameter_maker_tag{};


	template < typename Tuple >
	struct iop_list{
	public:
		constexpr iop_list(Tuple const& tuple): tuple(tuple) {}

		template < char ... C >
		constexpr auto const& operator()(
			input_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple, [name](auto const& maker){
				return hana::is_a< input_maker_tag >(maker)
					&& maker.name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested input doesn't exist (yet)");
			return result.value();
		}

		template < char ... C >
		constexpr auto const& operator()(
			output_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple, [name](auto const& maker){
				return hana::is_a< output_maker_tag >(maker)
					&& maker.name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested output doesn't exist (yet)");
			return result.value();
		}

		template < char ... C >
		constexpr auto const& operator()(
			parameter_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple, [name](auto const& maker){
				return hana::is_a< parameter_maker_tag >(maker)
					&& maker.name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested parameter doesn't exist (yet)");
			return result.value();
		}

	private:
		Tuple const& tuple;
	};


}


#endif
