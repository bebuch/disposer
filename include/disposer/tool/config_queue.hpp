//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__config_queue__hpp_INCLUDED_
#define _disposer__tool__config_queue__hpp_INCLUDED_

#include <boost/hana/tuple.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/minus.hpp>


namespace disposer::detail{


	template < std::size_t Offset, typename ... Configs >
	class config_queue{
	public:
		static constexpr auto offset = hana::size_c< Offset >;

		static constexpr auto size = hana::size_c< sizeof...(Configs) > - offset;

		static constexpr auto is_empty = size() == hana::size_c< 0 >;


		constexpr config_queue(hana::tuple< Configs ... > const& tuple)noexcept
			: tuple_(tuple) {}

		constexpr auto next()const noexcept{
			return config_queue< Offset + 1, Configs ... >{tuple_};
		}

		constexpr decltype(auto) front()const noexcept{
			return tuple_[offset];
		}


	private:
		hana::tuple< Configs ... > const& tuple_;
	};

	template < typename ... Configs >
	config_queue(hana::tuple< Configs ... >)
		-> config_queue< 0, Configs ... >;


}


#endif
