//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
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


namespace disposer::detail{


	template < typename Tuple, std::size_t Offset >
	class config_queue{
	public:
		static constexpr auto offset = hana::size_c< Offset >;

		template < typename ... Ts >
		constexpr config_queue(
			hana::tuple< Ts ... >& tuple,
			hana::size_t< Offset > = hana::size_c< 0 >
		)noexcept: tuple_(tuple) {}

		template < typename ... Ts >
		constexpr config_queue(
			hana::tuple< Ts ... > const& tuple,
			hana::size_t< Offset > = hana::size_c< 0 >
		)noexcept: tuple_(tuple) {}

		constexpr auto size()const noexcept{
			return hana::size(tuple_) - offset;
		}

		constexpr config_queue< Tuple, Offset + 1 > next()const noexcept{
			return {tuple_, hana::size_c< Offset + 1 >};
		}

		constexpr auto is_last()const noexcept{
			return size() == hana::size_c< 1 >;
		}

		constexpr decltype(auto) front()const noexcept{
			return tuple_[hana::size_c< 0 >];
		}


	private:
		Tuple& tuple_;
	};

	template < typename ... Ts, std::size_t Offset >
	config_queue(hana::tuple< Ts ... >& tuple, hana::size_t< Offset >)
		-> config_queue< hana::tuple< Ts ... >, Offset >;

	template < typename ... Ts, std::size_t Offset >
	config_queue(hana::tuple< Ts ... > const& tuple, hana::size_t< Offset >)
		-> config_queue< hana::tuple< Ts ... > const, Offset >;


}


#endif
