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
		constexpr auto operator()(T& name)const noexcept{
			return std::ref(name);
		}
	};


}


namespace disposer{


	using namespace hana = boost::hana;


	template < typename List, std::size_t I >
	constexpr auto get_name
		= decltype(std::declval< List >()[hana::size_c< I >].name)();

	template < typename List, std::size_t I, typename Module >
	auto io_exec_make_data(
		Module const& module,
		output_map_type const& output_map
	)noexcept{
		constexpr auto name = get_name< List, I >;
		if constexpr(hana::is_a< input_name_tag >(name)){
			auto const ptr = module(name).output_ptr();
			return ptr ? output_map[ptr] : nullptr;
		}else if constexpr(hana::is_a< output_name_tag >(name)){
			return module(name).use_count();
		}else{
			static_assert(false_c< name_type >,
				"name must be an input_name or an output_name");
		}
	}

	template < typename List, std::size_t I, typename Module, typename Data >
	auto add_outputs_to_map(
		Module const& module,
		Data const& data,
		output_map_type& output_map
	){
		constexpr auto name = get_name< List, I >;
		if constexpr(hana::is_a< output_name_tag >(name)){
			output_map.emplace(&(module(name)), &(data(name)));
		}
	}


	/// \brief Accessory of a \ref module without log
	template < typename List >
	class module_exec_data{
	public:
		/// \brief Constructor
		template < typename Module, std::size_t ... I >
		module_exec_data(
			Module const& module,
			output_map_type& output_map,
			std::index_sequence< I ... >
		)
			: list_(io_exec_make_data< List, I >(module, output_map) ...)
		{
			(add_output_to_map< List, I >(module, list_, output_map), ...);
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
