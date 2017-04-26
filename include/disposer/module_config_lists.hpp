//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_config_lists__hpp_INCLUDED_
#define _disposer__module_config_lists__hpp_INCLUDED_

#include <boost/hana.hpp>


namespace disposer{


	namespace hana = ::boost::hana;


	/// \brief Tag for in_t
	struct in_tag{};

	/// \brief Tag for out_t
	struct out_tag{};

	/// \brief Tag for param_t
	struct param_tag{};


	/// \brief Create three boost::hana::map's from the given input, output
	///        and parameter types
	template < typename ... ConfigList >
	constexpr auto separate_module_config_lists(ConfigList&& ...){
		constexpr auto types = hana::make_tuple(hana::type_c<
			std::remove_cv_t< std::remove_reference_t< ConfigList > > > ...);

		static_assert(hana::all_of(types, [](auto&& t){
			using type = typename decltype(+t)::type;
			return
				hana::is_a< in_tag, type > ||
				hana::is_a< out_tag, type > ||
				hana::is_a< param_tag, type >;
		}), "only inputs, outputs and parameters are allowed as argument");

		constexpr auto inputs = hana::filter(types, [](auto&& type){
				return hana::is_a< in_tag, typename decltype(+type)::type >;
			});
		constexpr auto outputs = hana::filter(types, [](auto&& type){
				return hana::is_a< out_tag, typename decltype(+type)::type >;
			});
		constexpr auto parameters = hana::filter(types, [](auto&& type){
				return hana::is_a< param_tag, typename decltype(+type)::type >;
			});

		constexpr auto io_type_list = [](auto ... io){
			return hana::type_c< hana::map< hana::pair<
				typename decltype(+io)::type::name_type::value_type,
				typename decltype(+io)::type::type > ... > >;
		};

		return hana::make_tuple(
			hana::unpack(inputs, io_type_list),
			hana::unpack(outputs, io_type_list),
			hana::unpack(parameters, io_type_list));
	}


}


#endif
