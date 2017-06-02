//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__id_increase__hpp_INCLUDED_
#define _disposer__id_increase__hpp_INCLUDED_

#include <type_traits>
#include <stdexcept>
#include <cstdint>


namespace disposer{


	struct id_increase_t{
		std::size_t const expand;
		std::size_t const reduce;
	};

	struct normal_id_increase{
		template < typename IOP_List >
		constexpr id_increase_t operator()(
			IOP_List const& /* iop_list */
		)const noexcept{
			return {1, 1};
		}
	};

	struct id_increase_fn_tag;

	template < typename Fn >
	class id_increase_fn{
	public:
		using hana_tag = id_increase_fn_tag;

		constexpr id_increase_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr id_increase_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&), id_increase_t >,
				"Wrong function signature, expected: "
				"id_increase_t f(auto const& iop)"
			);
#else
			static_assert(
				std::is_invocable_r_v< id_increase_t, Fn const,
					IOP_List const& >,
				"Wrong function signature, expected: "
				"id_increase_t f(auto const& iop)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >()
			));
		}

		template < typename IOP_List >
		constexpr id_increase_t operator()(
			IOP_List const& iop_list
		)const noexcept(calc_noexcept< IOP_List >()){
			std::size_t expand = 0;
			std::size_t reduce = 0;
			iop_list.log([&expand, &reduce](logsys::stdlogb& os){
					os << "with expand = " << expand
						<< " and reduce = " << reduce;
				},
				[&]{
					id_increase_t result = fn_(iop_list);
					expand = result.expand;
					reduce = result.reduce;
					if(expand == 0 || reduce == 0){
						throw std::logic_error(
							"id increase expand and reduce must not be 0");
					}
				});
			return id_increase_t{expand, reduce};
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto id_increase(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return id_increase_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}


}


#endif
