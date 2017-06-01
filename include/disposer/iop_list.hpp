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

#include "add_log.hpp"
#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"

#include <iostream>


namespace disposer{


	/// \brief Tag for input_maker
	struct input_maker_tag{};

	/// \brief Tag for output_maker
	struct output_maker_tag{};

	/// \brief Tag for parameter_maker
	struct parameter_maker_tag{};


	/// \brief Tag for input
	struct input_tag{};

	/// \brief Tag for output
	struct output_tag{};

	/// \brief Tag for parameter
	struct parameter_tag{};


	struct as_reference_list{
		template < typename T >
		constexpr auto operator()(T const& maker)const noexcept{
			return std::cref(maker);
		}
	};


	template < typename LogFn, typename Tuple >
	struct iop_list: add_log< iop_list< LogFn, Tuple > >{
	public:
		constexpr iop_list(LogFn const& log_fn, Tuple const& tuple)noexcept
			: log_fn_(log_fn)
			, tuple_(hana::transform(tuple, as_reference_list{})) {}

		template < char ... C >
		constexpr auto const& operator()(
			input_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple_, [name](auto const& maker){
				return hana::is_a< input_tag >(maker.get())
					&& maker.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested input doesn't exist (yet)");
			return result.value().get();
		}

		template < char ... C >
		constexpr auto const& operator()(
			output_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple_, [name](auto const& maker){
				return hana::is_a< output_tag >(maker.get())
					&& maker.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested output doesn't exist (yet)");
			return result.value().get();
		}

		template < char ... C >
		constexpr auto const& operator()(
			parameter_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple_, [name](auto const& maker){
				return hana::is_a< parameter_tag >(maker.get())
					&& maker.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested parameter doesn't exist (yet)");
			return result.value().get();
		}

		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			log_fn_(os);
		}

	private:
		LogFn const& log_fn_;

		decltype(hana::transform(std::declval< Tuple >(), as_reference_list{}))
			tuple_;
	};

	template < typename LogFn, typename Tuple >
	constexpr auto make_iop_list(
		LogFn const& log_fn,
		Tuple const& tuple
	)noexcept{
		return iop_list< LogFn, Tuple >(log_fn, tuple);
	}


}


#endif
