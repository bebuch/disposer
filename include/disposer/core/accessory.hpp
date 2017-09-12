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
	};

	template < typename ... IOP_RefList >
	struct iops_ref{};

	iops_ref() -> iops_ref<>;

	template < typename IOP_Ref, typename ... IOP_RefList >
	struct iops_ref< IOP_Ref, IOP_RefList ... >{
		IOP_Ref const& ref;
		iops_ref< IOP_RefList ... > const& list;

		iops_ref(IOP_Ref const& ref, iops_ref< IOP_RefList ... > const& list)
			: ref(ref), list(list) {}

		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			if constexpr(ref.name == name){
				return ref;
			}else{
				static_assert(sizeof...(IOP_RefList) > 0,
					"object with name is unknown");

				list(name);
			}
		}
	};

	template < typename ... IOP_RefList >
	class iops_accessory: public add_log< iops_accessory< IOP_RefList ... > >{
	public:
		iops_accessory(
			iops_ref< IOP_RefList ... > const& list,
			iop_log&& log_fn
		)noexcept
			: iop_list_(iop_list)
			, log_fn_(std::move(log_fn)) {}


		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return list(name);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << log_fn_.location << " " << log_fn_.maker_type_name
				<< "(" << log_fn_.maker_name << ") ";
		}


	private:
		/// \brief References to all previous IOPs
		iops_ref< IOP_RefList ... > const& list;

		/// \brief Reference to an iop_log object
		iop_log log_fn_;
	};


}


#endif
