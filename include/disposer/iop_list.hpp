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


	/// \brief Hana Tag for input_maker
	struct input_maker_tag{};

	/// \brief Hana Tag for output_maker
	struct output_maker_tag{};

	/// \brief Hana Tag for parameter_maker
	struct parameter_maker_tag{};


	/// \brief Hana Tag for input
	struct input_tag{};

	/// \brief Hana Tag for output
	struct output_tag{};

	/// \brief Hana Tag for parameter
	struct parameter_tag{};


	/// \brief std::cref as callable object
	struct as_reference_list{
		template < typename T >
		constexpr auto operator()(T const& iop)const noexcept{
			return std::cref(iop);
		}
	};


	/// \brief Log Implementation for \ref iop_ref
	struct iop_log{
		std::string_view const location;
		std::string_view const maker_type_name;
		std::string_view const maker_name;

		void operator()(logsys::stdlogb& os)const{
			os << location << " " << maker_type_name << "(" << maker_name
				<< ") ";
		}
	};


	/// \brief Accessory object for all former IOP-object
	template < typename IOP_Tuple >
	struct iop_list: add_log< iop_list< IOP_Tuple > >{
	public:
		/// \brief Constructor
		constexpr iop_list(
			iop_log const& log_fn,
			IOP_Tuple const& iop_tuple
		)noexcept
			: log_fn_(log_fn)
			, iop_tuple_(hana::transform(iop_tuple, as_reference_list{})) {}

		/// \brief Get input by compile time name
		template < char ... C >
		constexpr auto const& operator()(
			input_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(iop_tuple_, [name](auto const& input){
				return hana::is_a< input_tag >(input.get())
					&& input.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested input doesn't exist (yet)");
			return result.value().get();
		}

		/// \brief Get output by compile time name
		template < char ... C >
		constexpr auto const& operator()(
			output_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(iop_tuple_, [name](auto const& output){
				return hana::is_a< output_tag >(output.get())
					&& output.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested output doesn't exist (yet)");
			return result.value().get();
		}

		/// \brief Get parameter by compile time name
		template < char ... C >
		constexpr auto const& operator()(
			parameter_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(iop_tuple_, [name](auto const& param){
				return hana::is_a< parameter_tag >(param.get())
					&& param.get().name == name.value;
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
		/// \brief Reference to an iop_log object
		iop_log const& log_fn_;

		/// \brief hana::tuple of std::reference_wrapper's of all previos iop's
		///
		/// The conversion to std::reference_wrapper is necessary because of
		/// hana::optional which can not holf a raw reference.
		decltype(hana::transform(std::declval< IOP_Tuple >(),
			as_reference_list{})) iop_tuple_;
	};

	/// \brief Maker function for \ref iop_list
	template < typename IOP_Tuple >
	constexpr auto make_iop_list(
		iop_log const& log_fn,
		IOP_Tuple const& iop_tuple
	)noexcept{
		return iop_list< IOP_Tuple >(log_fn, iop_tuple);
	}


}


#endif
