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

#include "../tool/depended_t.hpp"


namespace disposer{



	template < typename ... T >
	std::string get_type_name(std::size_t i, dimension< T ... >){
		using hana::literals;
		// TODO; Find a more efficent solution …
		std::string name;
		hana::while_(
			hana::less.than(hana::size_c< sizeof...(T) >), 0_c, [&](auto I){
				if(i == I){
					name = type_index::type_id< typename decltype(hana::arg
							< I + 1 >(hana::basic_type< T > ...))::type
						>().pretty_name();
				}
				return I + 1_c;
			});

#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
		assert(name != "");
#endif

		return name;
	}



	/// \brief Type referes to at least one dimension list of a module
	template < std::size_t D, std::size_t ... Ds >
	struct dim_numbers{};

	/// \brief Tag for set_dimension_fn
	struct set_dimension_fn_tag;

	/// \brief Chose a type from at least one dimension_list
	template < typename Fn, std::size_t ... D >
	class set_dimension_fn{
	public:
		/// \brief Hana tag to identify set_dimension_fn
		using hana_tag = set_dimension_fn_tag;

		/// \brief List of numbers refering to the dimension_list
		static constexpr auto dims = dim_numbers< D ... >;

		/// \brief Tuple with the same count of size_t's as D's
		using result_type =
			std::tuple< detail::value_depended_t< std::size_t, D > ... >;


		/// \brief Default construtor
		constexpr set_dimension_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		/// \brief construtor
		constexpr explicit set_dimension_fn(dim_numbers< D ... >, Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		/// \brief construtor
		constexpr explicit set_dimension_fn(dim_numbers< D ... >, Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		/// \brief Helper for noexcept of operator()
		template < typename Accessory, typename ... Dimensions >
		static constexpr bool calc_noexcept()noexcept{
#ifdef DISPOSER_CONFIG_ENABLE_DEBUG_MODE
			static_assert(sizeof...(D) == sizeof...(Dimensions));
			static_assert(hana::all_of(
				std::declval< hana::basic_tuple< Dimension ... > >()),
				hana::is_a< dimension_tag >);
#endif

			static_assert(std::is_invocable_r_v<
				result_type,
				Fn const, Accessory const& >,
				"Wrong function signature, expected: "
				"std::size_t f(auto const& iop)"
			);

			return std::is_nothrow_invocable_v< Fn const, Accessory const& >;
		}

		/// \brief Calls the actual function
		template < typename Accessory, typename ... Dimensions >
		result_type operator()(
			Accessory const& accessory,
			Dimensions const& ... dims
		)const{
			return accessory.log(
				[](logsys::stdlogb& os, result_type* indexes){
					os << "set dimension number";
					if constexpr(sizeof...(D) == 1) os << "s";
					os << " ";

					if(!indexes){
						detail::comma_separated_output(os, D ...);
						return;
					}

					std::apply(*indices, [](std::size_t ... i){
						detail::comma_separated_output(os, std::tuple(
							D, " to ", get_type_name(i, Dimensions{})) ...);
					});
				}, [&]{
					result_type indices = std::invoke(fn_, accessory);
					std::apply(*indices, [](std::size_t ... i){
						if((i < Dimensions::type_count && ...)) return;
						std::ostringstream os;
						os << "index is out of range: ";
						std::apply(*indices, [](std::size_t ... i){
							using namespace std::literals::string_view_literals;
							detail::comma_separated_output(os, std::tuple(
								"dimension number ", D, " has ",
								Dimensions::type_count, " types (index ",
								i, " is ", (i < Dimensions::type_count
									? "valid"sv : "invalid"sv), ")");
						});
						throw std::out_of_range(os.str());
					});
					return indices;
				});
		}

	private:
		Fn fn_;
	};


}


#endif
