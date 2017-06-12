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


	/// \brief Information about the ID increase of a module
	struct id_increase_t{
		/// \brief How much ID's does the module fire after getting data
		std::size_t const expand;

		/// \brief How much ID's does the module consume before firing
		std::size_t const reduce;
	};

	/// \brief Default class for a module which fires one times per ID
	struct normal_id_increase{
		template < typename IOP_List >
		constexpr id_increase_t operator()(
			IOP_List const& /* iop_list */
		)const noexcept{
			return {1, 1};
		}
	};

	/// \brief Wrapper around a id_increase function
	///
	/// Needed for logging the results of the function.
	template < typename Fn >
	class id_increase_fn{
	public:
		/// \brief Copy in the increase function
		constexpr id_increase_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		/// \brief Move in the increase function
		constexpr id_increase_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		/// \brief Calculate the noexcept of the increase function
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

		/// \brief Call the increase function and log the result
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
		/// \brief The increase function
		Fn fn_;
	};

	/// \brief Maker function for \ref id_increase_fn
	template < typename Fn >
	constexpr auto id_increase(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return id_increase_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}


}


#endif
