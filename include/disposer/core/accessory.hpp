//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__accessory__hpp_INCLUDED_
#define _disposer__core__accessory__hpp_INCLUDED_

#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"
#include "dimension.hpp"
#include "exec_fn.hpp"

#include "../tool/add_log.hpp"
#include "../tool/extract.hpp"
#include "../tool/false_c.hpp"
#include "../tool/to_std_string.hpp"
#include "../tool/to_std_string_view.hpp"

#include <boost/hana/map.hpp>


namespace disposer{


	template < typename ... RefList >
	class iops_ref{
	public:
		hana::tuple<> flat()&&{ return {}; }
	};

	iops_ref() -> iops_ref<>;

	template < typename IOP_Ref, typename ... RefList >
	class iops_ref< IOP_Ref, RefList ... >{
	public:
		iops_ref(IOP_Ref&& ref, iops_ref< RefList ... >&& list)
			: ref(std::move(ref)), list(std::move(list)) {}

		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		decltype(auto) operator()(Name const& name)const noexcept{
			using name_t = std::remove_reference_t< Name >;
			if constexpr(IOP_Ref::name == name_t{}){
				if constexpr(hana::is_a< parameter_name_tag, name_t >()){
					return ref.get();
				}else if constexpr(hana::is_a< output_name_tag, name_t >()){
					return ref.use_count() > 0;
				}else{
					return ref.output_ptr() != nullptr;
				}
			}else{
				static_assert(sizeof...(RefList) > 0,
					"object with name is unknown");

				return list(name);
			}
		}

		auto flat()&&{
			return std::move(*this).flat(
				std::make_index_sequence< sizeof...(RefList) + 1 >());
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
				.template get< sizeof...(RefList) - Is >() ...);
		}

		IOP_Ref&& ref;
		iops_ref< RefList ... >&& list;

		template < typename ... OtherRefList >
		friend class iops_ref;
	};

	template < typename IOP_Ref, typename ... RefList >
	iops_ref(IOP_Ref&& ref, iops_ref< RefList ... >&& list)
		-> iops_ref< IOP_Ref, RefList ... >;


	template <
		typename Component,
		typename DimensionList,
		typename ... RefList >
	class module_make_accessory
		: public optional_component< Component >
		, public add_log<
			module_make_accessory< Component, DimensionList, RefList ... > >{
	public:
		using dimension_list = DimensionList;

		module_make_accessory(
			optional_component< Component > component,
			DimensionList,
			iops_ref< RefList ... > const& list,
			std::string_view location
		)noexcept
			: optional_component< Component >(component)
			, location(location)
			, list_(list) {}

		module_make_accessory(module_make_accessory const& other)noexcept
			: optional_component< Component >(other)
			, location(other.location)
			, list_(other.list_) {}


		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		decltype(auto) operator()(Name const& name)const noexcept{
			static_assert(
				hana::is_a< input_name_tag, Name >() ||
				hana::is_a< output_name_tag, Name >() ||
				hana::is_a< parameter_name_tag, Name >(),
				"name must be an input_name, an output_name or a "
				"parameter_name");
			return list_(name);
		}

		/// \brief Get type by dimension index
		template < std::size_t DI >
		static constexpr auto dimension(hana::size_t< DI > i)noexcept{
			static_assert(DI < DimensionList::dimension_count,
				"module has less then DI dimensions");
			static_assert(hana::size_c< 1 > ==
				hana::size(DimensionList::dimensions[hana::size_c< DI >]),
				"module dimension DI is not solved yet");
			return DimensionList::dimensions[i][hana::size_c< 0 >];
		}

		/// \brief Get all types of an unsolved dimension by index
		template < std::size_t DI >
		static constexpr auto dimension_types(hana::size_t< DI > i)noexcept{
			static_assert(DI < DimensionList::dimension_count,
				"module has less then DI dimensions");
			static_assert(hana::size_c< 1 > <
				hana::size(DimensionList::dimensions[hana::size_c< DI >]),
				"module dimension DI is already solved");
			return DimensionList::dimensions[i];
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location;
		}

		/// \brief Log location
		std::string_view location;


	private:
		/// \brief References to all previous IOPs
		iops_ref< RefList ... > const& list_;
	};


	template < typename DimensionList, typename ... RefList >
	class component_make_accessory
		: public add_log<
			component_make_accessory< DimensionList, RefList ... > >{
	public:
		using dimension_list = DimensionList;

		component_make_accessory(
			DimensionList,
			iops_ref< RefList ... > const& list,
			std::string_view location
		)noexcept: location(location), list_(list) {}

		component_make_accessory(component_make_accessory const& other)noexcept
			: location(other.location)
			, list_(other.list_) {}


		/// \brief Get const reference to an parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		decltype(auto) operator()(Name const& name)const noexcept{
			static_assert(hana::is_a< parameter_name_tag, Name >(),
				"name must be a parameter_name");
			return list_(name);
		}

		/// \brief Get type by dimension index
		template < std::size_t DI >
		static constexpr auto dimension(hana::size_t< DI > i)noexcept{
			static_assert(DI < DimensionList::dimension_count,
				"component has less then DI dimensions");
			static_assert(hana::size_c< 1 > ==
				hana::size(DimensionList::dimensions[hana::size_c< DI >]),
				"component dimension DI is not solved yet");
			return DimensionList::dimensions[i][hana::size_c< 0 >];
		}

		/// \brief Get all types of an unsolved dimension by index
		template < std::size_t DI >
		static constexpr auto dimension_types(hana::size_t< DI > i)noexcept{
			static_assert(DI < DimensionList::dimension_count,
				"component has less then DI dimensions");
			static_assert(hana::size_c< 1 > <
				hana::size(DimensionList::dimensions[hana::size_c< DI >]),
				"component dimension DI is already solved");
			return DimensionList::dimensions[i];
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			os << location;
		}

		/// \brief Log location
		std::string_view location;


	private:
		/// \brief References to all previous IOPs
		iops_ref< RefList ... > const& list_;
	};


}


#endif
