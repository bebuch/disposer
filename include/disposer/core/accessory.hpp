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
#include <boost/hana/tuple.hpp>


namespace disposer{


	template < typename ... IOP_RefList >
	class iops_ref{
	public:
		hana::tuple<> flat()&&{ return {}; }
	};

	iops_ref() -> iops_ref<>;

	template < typename IOP_Ref, typename ... IOP_RefList >
	class iops_ref< IOP_Ref, IOP_RefList ... >{
	public:
		iops_ref(IOP_Ref&& ref, iops_ref< IOP_RefList ... >&& list)
			: ref(std::move(ref)), list(std::move(list)) {}

		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			if constexpr(IOP_Ref::name == name){
				return ref;
			}else{
				static_assert(sizeof...(IOP_RefList) > 0,
					"object with name is unknown");

				list(name);
			}
		}

		auto flat()&&{
			return std::move(*this).flat(
				std::make_index_sequence< sizeof...(IOP_RefList) + 1 >());
		}

	private:
		template < std::size_t I >
		auto&& get()&&{
			if constexpr(I == 0){
				return std::move(ref);
			}else{
				return std::move(list).template get< I - 1 >();
			}
		}

		template < std::size_t ... Is >
		auto flat(std::index_sequence< Is ... >)&&{
			return hana::make_tuple(std::move(*this)
				.template get< sizeof...(IOP_RefList) - Is >() ...);
		}

		IOP_Ref&& ref;
		iops_ref< IOP_RefList ... >&& list;

		template < typename ... OtherIOP_RefList >
		friend class iops_ref;
	};

	template < typename IOP_Ref, typename ... IOP_RefList >
	iops_ref(IOP_Ref&& ref, iops_ref< IOP_RefList ... >&& list)
		-> iops_ref< IOP_Ref, IOP_RefList ... >;

	template < typename ... IOP_RefList >
	class iops_accessory: public add_log< iops_accessory< IOP_RefList ... > >{
	public:
		iops_accessory(
			iops_ref< IOP_RefList ... > const& list,
			std::string_view log_fn
		)noexcept
			: list_(list)
			, log_fn_(log_fn) {}


		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return list_(name);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << log_fn_;
		}


	private:
		/// \brief References to all previous IOPs
		iops_ref< IOP_RefList ... > const& list_;

		/// \brief Log location
		std::string_view log_fn_;
	};


}


#endif
