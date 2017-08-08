//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_exec_data__hpp_INCLUDED_
#define _disposer__core__module_exec_data__hpp_INCLUDED_

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/hana/slice.hpp>
#include <boost/hana/size.hpp>

#include <type_traits>
#include <string_view>
#include <functional>
#include <utility>
#include <vector>


namespace disposer::detail{


	/// \brief std::ref as callable object
	struct ref{
		template < typename T >
		constexpr auto operator()(T& name)noexcept const{
			return std::ref(name);
		}
	};


}


namespace disposer{


	using namespace hana = boost::hana;


	/// \brief Accessory of a \ref module without log
	template < typename List >
	class module_exec_data{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData, std::size_t ... I >
		module_exec_data(
			MakerList const& maker_list,
			MakeData const& data,
			std::string_view location,
			std::index_sequence< I ... >
		)
			: list_(iops_make_data(
				iop_make_data(maker_list[hana::size_c< I >], data, location),
				location, hana::slice_c< 0, I >(ref_list()),
				hana::size_c< I >) ...)
		{
			(void)location; // GCC bug (silance unused warning)
		}


		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return get(name);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)noexcept const{
			return get(name);
		}


	private:
		/// \brief list_ as tuple of std::reference_wrapper's
		auto ref_list()noexcept const{
			return hana::transform(list_, detail::ref{});
		}

		/// \brief Implementation for \ref operator()
		template < typename Name >
		auto& get(Name const& name)noexcept const{
			using name_t = std::remove_reference_t< Name >;
			static_assert(
				hana::is_a< input_name_tag, name_t > ||
				hana::is_a< output_name_tag, name_t >,
				"parameter is not an input_name or output_name");

			using name_tag = typename name_t::hana_tag;

			auto ref = hana::find_if(ref_list(), [&name](auto ref){
					using tag = typename decltype(ref)::type::name_type
						::hana_tag;
					return hana::type_c< name_tag > == hana::type_c< tag >
						&& ref.get().name == name.value;
				});

			auto is_iop_valid = ref != hana::nothing;
			static_assert(is_iop_valid, "requested name doesn't exist");

			return ref->get();
		}

		/// \brief hana::tuple of the inputs and outputs
		List list_;
	};


}


#endif
