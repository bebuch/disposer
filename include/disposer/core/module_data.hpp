//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_data__hpp_INCLUDED_
#define _disposer__core__module_data__hpp_INCLUDED_

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
		constexpr auto operator()(T& iop)noexcept const{
			return std::ref(iop);
		}
	};


}


namespace disposer{


	using namespace hana = boost::hana;


	/// \brief Accessory of a \ref module without log
	template < typename List >
	class module_data{
	public:
		/// \brief Constructor
		template < typename MakerList, typename MakeData, std::size_t ... I >
		module_data(
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
		template < typename IOP >
		auto& operator()(IOP const& iop)noexcept{
			return get(iop);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename IOP >
		auto const& operator()(IOP const& iop)noexcept const{
			return get(iop);
		}


	private:
		/// \brief list_ as tuple of std::reference_wrapper's
		constexpr auto ref_list()noexcept const{
			return hana::transform(list_, detail::ref{});
		}

		/// \brief Implementation for \ref operator()
		template < typename IOP >
		constexpr auto& get(IOP const& iop)noexcept const{
			using iop_t = std::remove_reference_t< IOP >;
			static_assert(
				hana::is_a< input_name_tag, iop_t > ||
				hana::is_a< output_name_tag, iop_t > ||
				hana::is_a< parameter_name_tag, iop_t >,
				"parameter is not an input_name, output_name or "
				"parameter_name");

			using iop_tag = typename iop_t::hana_tag;

			auto iop_ref = hana::find_if(ref_list(), [&iop](auto ref){
					using tag = typename decltype(ref)::type::name_type
						::hana_tag;
					return hana::type_c< iop_tag > == hana::type_c< tag >
						&& ref.get().name == iop.value;
				});

			auto is_iop_valid = iop_ref != hana::nothing;
			static_assert(is_iop_valid, "requested iop doesn't exist");

			return iop_ref->get();
		}

		/// \brief hana::tuple of the inputs, outputs and parameters
		List list_;


		/// \brief std::vector with references to all input's (input_base)
		friend auto generate_input_list(module_data& config){
			auto input_ref_list = hana::filter(config.ref_list(), [](auto ref){
				return hana::is_a< input_tag >(ref.get());
			});
			return hana::unpack(input_ref_list, [](auto ... input_ref){
				return std::vector< std::reference_wrapper< input_base > >{
					std::reference_wrapper< input_base >(input_ref) ... };
			});
		}

		/// \brief std::vector with references to all output's (output_base)
		friend auto generate_output_list(module_data& config){
			auto output_ref_list = hana::filter(config.ref_list(), [](auto ref){
				return hana::is_a< output_tag >(ref.get());
			});
			return hana::unpack(output_ref_list, [](auto ... output_ref){
				return std::vector< std::reference_wrapper< output_base > >{
					std::reference_wrapper< output_base >(output_ref) ... };
			});
		}
	};


}


#endif
