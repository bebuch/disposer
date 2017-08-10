//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__extract__hpp_INCLUDED_
#define _disposer__tool__extract__hpp_INCLUDED_

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/find_if.hpp>

#include <utility>


namespace disposer{


	namespace hana = boost::hana;


}


namespace disposer::detail{


	/// \brief std::ref as callable object
	struct ref{
		template < typename T >
		constexpr auto operator()(T& name)const noexcept{
			return std::ref(name);
		}
	};


	/// \brief list_ as tuple of std::reference_wrapper's
	template < typename List >
	auto as_ref_list(List& list)noexcept{
		return hana::transform(list, ref{});
	}


	/// \brief Implementation for \ref operator()
	template < typename RefList, typename Name >
	auto& extract(RefList& list, Name const& name)noexcept{
		using name_t = std::remove_reference_t< Name >;
		using name_tag = typename name_t::hana_tag;

		auto ref = hana::find_if(list, [&name](auto ref){
				return hana::is_a< name_tag >(ref.get().name)
					&& ref.get().name.value == name.value;
			});

		auto is_iop_valid = ref != hana::nothing;
		static_assert(is_iop_valid, "requested name doesn't exist");

		return ref->get();
	}


}


#endif
