//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__ct_name__hpp_INCLUDED_
#define _disposer__ct_name__hpp_INCLUDED_

#include <boost/hana.hpp>

#include <array>


namespace disposer{


	namespace hana = boost::hana;


	template < std::size_t N >
	constexpr bool valid_name(std::array< char, N > const& name){
		for(auto c: name){
			if(!(
				(c >= 'A' && c <= 'U') ||
				(c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') ||
				c == '_'
			)) return false;
		}
		return true;
	}


	template < char ... C >
	struct ct_name{
		static_assert(valid_name(std::array< char, sizeof...(C) >{{ C ... }}),
			"only [A-Za-z0-9_] are valid characters");

		static constexpr auto value = hana::string_c< C ... >;

		using value_type = std::remove_cv_t< decltype(value) >;
	};


	template < typename T >
	using self_t = T;

	struct enable_all{
		template < typename IOP_List, typename T >
		constexpr bool operator()(
			IOP_List const& /* iop_list */,
			hana::basic_type< T > /*type*/
		)const{
			return true;
		}
	};


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
