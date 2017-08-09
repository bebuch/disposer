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

#include <unordered_map>
#include <type_traits>
#include <string_view>
#include <functional>
#include <utility>
#include <vector>


namespace disposer::detail{


	/// \brief std::ref as callable object
	struct ref{
		template < typename T >
		constexpr auto operator()(T& name)const noexcept{
			return std::ref(name);
		}
	};


}


namespace disposer{


	using namespace hana = boost::hana;


	using output_name_to_ptr_type
		= std::unordered_map< std::string, output_base* >;


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
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return get(name);
		}

		/// \brief Get reference to an input-, output- or parameter-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return get(name);
		}


		output_name_to_ptr_type output_name_to_ptr()const{
			output_name_to_ptr_type map;
			hana::for_each(ref_list(), [&map](auto ref){
					if constexpr(hana::is_a< output_name_tag >(ref.get().name)){
						map.emplace(
							detail::to_std_string(ref.get().name),
							&ref.get());
					}
				});
			return map;
		}


	private:
		/// \brief list_ as tuple of std::reference_wrapper's
		auto ref_list()const noexcept{
			return hana::transform(list_, detail::ref{});
		}

		/// \brief Implementation for \ref operator()
		template < typename Name >
		auto& get(Name const& name)const noexcept{
			using name_t = std::remove_reference_t< Name >;
			static_assert(
				hana::is_a< input_name_tag, name_t > ||
				hana::is_a< output_name_tag, name_t > ||
				hana::is_a< parameter_name_tag, name_t >,
				"parameter is not an input_name, output_name or "
				"parameter_name");

			using name_tag = typename name_t::hana_tag;

			auto ref = hana::find_if(ref_list(), [&name](auto ref){
					return hana::is_a< name_tag >(ref.get().name)
						&& ref.get().name == name.value;
				});

			auto is_iop_valid = ref != hana::nothing;
			static_assert(is_iop_valid, "requested name doesn't exist");

			return ref->get();
		}

		/// \brief hana::tuple of the inputs, outputs and parameters
		List list_;
	};


}


#endif
