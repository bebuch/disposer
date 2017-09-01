//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__accessory__hpp_INCLUDED_
#define _disposer__core__accessory__hpp_INCLUDED_

#include "../tool/add_log.hpp"
#include "../tool/extract.hpp"
#include "../tool/false_c.hpp"
#include "../tool/to_std_string.hpp"
#include "../tool/to_std_string_view.hpp"

#include <boost/hana/map.hpp>


namespace disposer{


	/// \brief Log Implementation for \ref iop_ref
	struct iop_log{
		std::string_view location;
		std::string_view maker_type_name;
		std::string_view maker_name;

		void operator()(logsys::stdlogb& os)const{
			os << location << " " << maker_type_name
				<< "(" << maker_name << ") ";
		}
	};


	using namespace std::literals::string_view_literals;


	template < typename IOP_RefList >
	class iops_accessory: public add_log< iops_accessory< IOP_RefList > >{
	public:
		iops_accessory(
			IOP_RefList const& iop_list,
			iop_log&& log_fn
		)noexcept
			: iop_list_(iop_list)
			, log_fn_(std::move(log_fn)) {}


		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			using name_t = std::remove_reference_t< Name >;
			static_assert(
				hana::is_a< input_name_tag, name_t > ||
				hana::is_a< output_name_tag, name_t > ||
				hana::is_a< parameter_name_tag, name_t >,
				"name is not an input_name, output_name or parameter_name");

			return detail::extract(iop_list_, name);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			log_fn_(os);
		}


	private:
		IOP_RefList iop_list_;

		/// \brief Reference to an iop_log object
		iop_log log_fn_;
	};

	template < typename IOP_RefList, typename MakeData >
	struct iops_make_data{
		iops_make_data(
			MakeData&& make_data,
			std::string_view location,
			IOP_RefList const& iop_list
		)noexcept
			: data(static_cast< MakeData&& >(make_data))
			, accessory(iop_list, iop_log{
				location, make_data.log_name,
				detail::to_std_string_view(make_data.maker.name)}) {}

		MakeData data;
		iops_accessory< IOP_RefList > accessory;
	};


	template < typename ... Dimension, typename ... Config >
	auto make_module_make_data(
		dimension_list< Dimension ... >,
		module_configure< Config ... > const& configs,
		module_make_data const& data
	){
		return hana::fold(configs, hana::make_pair(
				undeduced_list_index_c< sizeof...(Dimension) >,
				hana::make_tuple()),
			[](auto&& state, auto const& config){
				return make_data(config, dimension_list< Dimension ... >{},
					data,
					hana::first(static_cast< decltype(state)&& >(state))
					hana::second(static_cast< decltype(state)&& >(state)));
			})
	};


}


#endif
