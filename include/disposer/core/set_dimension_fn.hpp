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


	template < typename Types >
	std::string get_type_name(std::size_t i, Types types){
		using type_count = decltype(hana::size(types));

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		assert(i < type_count::value);
#endif

		return hana::unpack(
			hana::make_range(hana::size_c< 0 >, hana::size(types)),
			[i](auto ... I){
				constexpr auto name_fn = [](auto I){
						return []{
								return type_index::type_id< typename
										decltype(+types[I])::type
									>().pretty_name();
							};
					};

				using fn_type = std::string(*)();
				constexpr fn_type names[type_count::value]
					= {static_cast< fn_type >(name_fn(decltype(I){})) ...};
				return names[i]();
			});
	}


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
		template < typename ... IOP_RefList >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(std::is_invocable_v< Fn,
					iops_accessory< IOP_RefList ... > const& >,
				"Wrong function signature, expected: "
				"solved_dimensions< Ds ... > f(auto const& iops)"
			);

			using result_type = std::invoke_result_t< Fn,
				iops_accessory< IOP_RefList ... > const& >;

			check_result_type< result_type >();

			return std::is_nothrow_invocable_v< Fn,
				iops_accessory< IOP_RefList ... > const& >;
		}

		/// \brief Calls the actual function
		template < typename Dimensions, typename ... IOP_RefList >
		auto operator()(
			Dimensions const&,
			iops_accessory< IOP_RefList ... > const& accessory
		)const noexcept(calc_noexcept< IOP_RefList ... >()){
			return accessory.log(
				[](logsys::stdlogb& os, auto* solved_dims_ptr){
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

					hana::unpack(solved_dims_ptr->indexes, [&os](auto ... ic){
						detail::comma_separated_output(os, std::make_tuple(ic.d,
							" to ", get_type_name(ic.i, Dimensions::dimensions[hana::size_c< ic.d >])) ...);
					});
				}, [&]{
					auto solved_dims = std::invoke(fn_, accessory);

					hana::unpack(solved_dims.indexes, [](auto ... ic){
						if(((ic.i < hana::size(Dimensions::dimensions[hana::size_c< ic.d >])) && ...)) return;

						std::ostringstream os;
						os << "index is out of range: ";

						using namespace std::literals::string_view_literals;

						detail::comma_separated_output(os, std::make_tuple(
							"dimension number ", ic.d, " has ",
							hana::size(Dimensions::dimensions[hana::size_c< ic.d >]).value, " types (index ",
							ic.i, " is ", (ic.i < hana::size(Dimensions::dimensions[hana::size_c< ic.d >])
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
