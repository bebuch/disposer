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


	template < typename Tuple >
	struct iop_list{
	public:
		constexpr iop_list(Tuple const& tuple)noexcept:
			tuple(hana::transform(tuple, as_reference_list{})) {}

		template < char ... C >
		constexpr auto const& operator()(
			input_name< C ... > const& name
		)const noexcept{
			auto result = hana::find_if(tuple, [name](auto const& maker){
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
			auto result = hana::find_if(tuple, [name](auto const& maker){
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
			auto result = hana::find_if(tuple, [name](auto const& maker){
				return hana::is_a< parameter_tag >(maker.get())
					&& maker.get().name == name.value;
			});
			static_assert(result != hana::nothing,
				"requested parameter doesn't exist (yet)");
			return result.value().get();
		}

	private:
		decltype(hana::transform(std::declval< Tuple >(), as_reference_list{}))
			tuple;
	};

	template < typename Tuple >
	constexpr auto make_iop_list(Tuple const& tuple)noexcept{
		return iop_list< Tuple >(tuple);
	}


	/// \brief Create a hana::tuple of hana::type's with a given hana::type or
	///        a hana::Sequence of hana::type's
	template < typename Types >
	constexpr auto to_typelist(Types const&)noexcept{
		if constexpr(hana::is_a< hana::type_tag, Types >){
			return hana::make_tuple(Types{});
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));
			return hana::to_tuple(Types{});
		}
	}


}


#endif
