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

#include "input_exec.hpp"
#include "output_exec.hpp"

#include "../tool/false_c.hpp"
#include "../tool/extract.hpp"

#include <boost/hana/transform.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/hana/slice.hpp>


namespace disposer{


	namespace hana = boost::hana;


	template < typename List, std::size_t I >
	constexpr auto get_name = std::remove_reference_t<
		decltype(std::declval< List >()[hana::size_c< I >].name) >();

	template < typename List, std::size_t I, typename Module >
	auto io_exec_make_data(
		Module& module,
		output_map_type const& output_map
	)noexcept{
		constexpr auto name = get_name< List, I >;
		if constexpr(hana::is_a< input_name_tag >(name)){
			auto const ptr = module(name).output_ptr();
			if(!ptr) return static_cast< output_exec_base* >(nullptr);

			auto const iter = output_map.find(ptr);
			assert(iter != output_map.end());
			return iter->second;
		}else if constexpr(hana::is_a< output_name_tag >(name)){
			return module(name).use_count();
		}else{
			static_assert(detail::false_c< List >,
				"name must be an input_name or an output_name");
		}
	}

	template < typename List, std::size_t I, typename Module, typename Data >
	void add_outputs_to_map(
		Module& module,
		Data& data,
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
			Module& module,
			output_map_type& output_map,
			std::index_sequence< I ... >
		)
			: list_(io_exec_make_data< List, I >(module, output_map) ...)
		{
			(add_outputs_to_map< List, I >(module, *this, output_map), ...);
		}


		/// \brief Get reference to an input- or output-object via
		///        its corresponding compile time name
		template < typename Name >
		auto& operator()(Name const& name)noexcept{
			return extract(list_, name);
		}

		/// \brief Get reference to an input- or output-object via
		///        its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			return extract(list_, name);
		}


		/// \brief Cleanup inputs
		void cleanup()noexcept{
			hana::for_each(detail::as_ref_list(list_), [](auto ref){
					auto const is_input
						= hana::is_a< input_name_tag >(ref.get().name);
					if constexpr(is_input) ref.get().cleanup();
				});
		}


	private:
		template < typename L, typename Name >
		static auto& extract(L& list, Name const& name)noexcept{
			using name_t = std::remove_reference_t< Name >;
			static_assert(
				hana::is_a< input_name_tag, name_t > ||
				hana::is_a< output_name_tag, name_t >,
				"name is not an input_name or output_name");
			return detail::extract(detail::as_ref_list(list), name);
		}

		/// \brief hana::tuple of the inputs and outputs
		List list_;
	};


}


#endif
