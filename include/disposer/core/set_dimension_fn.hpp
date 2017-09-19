//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__set_dimension_fn__hpp_INCLUDED_
#define _disposer__core__set_dimension_fn__hpp_INCLUDED_

#include "dimension.hpp"
#include "accessory.hpp"

#include "../tool/comma_separated_output.hpp"

#include <boost/hana/functional/arg.hpp>


namespace disposer{


	/// \brief Tag for set_dimension_fn
	struct set_dimension_fn_tag;

	/// \brief Chose a type from at least one dimension_list
	template < typename Fn >
	class set_dimension_fn{
	public:
		/// \brief Hana tag to identify set_dimension_fn
		using hana_tag = set_dimension_fn_tag;


		/// \brief Default construtor
		constexpr set_dimension_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		/// \brief Construtor
		constexpr explicit set_dimension_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		/// \brief Construtor
		constexpr explicit set_dimension_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		/// \brief Checks result type
		template < typename ResultType >
		static constexpr void check_result_type()noexcept{
			static_assert(is_solved_dimensions_v< ResultType >,
				"result of Fn(auto const& iops) in set_dimension_fn must be "
				"solved_dimensions< Ds ... > with sizeof...(Ds) > 0");

			static_assert(ResultType::index_count > 0,
				"result of Fn(auto const& iops) in set_dimension_fn must "
				"solve at least one dimension, you must return an object of "
				"solved_dimensions< Ds ... > with sizeof...(Ds) > 0");
		}

		/// \brief Helper for noexcept of operator()
		template < typename ... RefList >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(std::is_invocable_v< Fn,
					module_accessory< RefList ... > const& >,
				"Wrong function signature, expected: "
				"solved_dimensions< Ds ... > f(auto const& iops)"
			);

			using result_type = std::invoke_result_t< Fn,
				module_accessory< RefList ... > const& >;

			check_result_type< result_type >();

			return std::is_nothrow_invocable_v< Fn,
				module_accessory< RefList ... > const& >;
		}

		/// \brief Calls the actual function
		template < typename Dimensions, typename ... RefList >
		auto operator()(
			Dimensions const&,
			module_accessory< RefList ... > const& accessory
		)const noexcept(calc_noexcept< RefList ... >()){
			auto const types = [](auto ic){
				return Dimensions::dimensions[hana::size_c< ic.d >];
			};

			auto const type_count = [types](auto ic){
				return hana::size(types(ic));
			};

			return accessory.log(
				[types](logsys::stdlogb& os, auto* solved_dims_ptr){
					os << "set dimension number";

					using type =
						std::remove_pointer_t< decltype(solved_dims_ptr) >;
					if constexpr(type::index_count > 1) os << "s";

					os << " ";

					if(solved_dims_ptr == nullptr){
						hana::unpack(type::dimension_numbers(),
							[&os](auto ... d){
								detail::comma_separated_output(
									os, std::size_t(d) ...);
							});
						return;
					}

					hana::unpack(solved_dims_ptr->indexes,
						[&os, types](auto ... ic){
							detail::comma_separated_output(os,
								std::make_tuple(ic.d, " to ",
								detail::get_type_name(ic.i, types(ic))) ...);
						});
				}, [&]{
					auto solved_dims = std::invoke(fn_, accessory);

					hana::unpack(solved_dims.indexes, [type_count](auto ... ic){
						if(((ic.i < type_count(ic)) && ...)) return;

						std::ostringstream os;
						os << "index is out of range: ";

						using namespace std::literals::string_view_literals;

						detail::comma_separated_output(os, std::make_tuple(
							"dimension number ", ic.d, " has ",
							type_count(ic).value, " types (index ",
							ic.i, " is ", (ic.i < type_count(ic)
								? "valid"sv : "invalid"sv), ")") ...);

						throw std::out_of_range(os.str());
					});

					return solved_dims;
				});
		}

	private:
		Fn fn_;
	};


}


#endif
