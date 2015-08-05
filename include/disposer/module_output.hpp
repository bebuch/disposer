//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_module_output_hpp_INCLUDED_
#define _disposer_module_output_hpp_INCLUDED_

#include "module_input_base.hpp"
#include "module_output_base.hpp"
#include "module_output_data.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/type_traits.hpp>

#include <functional>


namespace disposer{


	namespace hana = boost::hana;

	using boost::typeindex::type_id_with_cvr;


	template < typename T, typename ... U >
	auto make_type_map(){
		return hana::make_map(
			hana::make_pair(hana::type< T >, false),
			hana::make_pair(hana::type< U >, false) ...
		);
	}

	template < typename T, typename ... U >
	struct type_t{
		decltype(make_type_map< T, U ... >()) map = make_type_map< T, U ... >();
	};


	template < typename T >
	class module_output_interface{
	public:
		using value_type = T;


		module_output_interface(signal_t& signal): signal_(signal) {}


		void operator()(std::size_t id, value_type&& value){
			trigger_signal(id, std::make_shared< module_output_data< value_type > >(std::move(value)));
		}

		void operator()(std::size_t id, value_type const& value){
			trigger_signal(id, std::make_shared< module_output_data< value_type > >(value));
		}

		void operator()(std::size_t id, module_output_data_ptr< value_type > const& value){
			trigger_signal(id, value);
		}


	private:
		signal_t& signal_;

		void trigger_signal(std::size_t id, module_output_data_ptr< value_type > const& value){
			signal_(id, reinterpret_cast< any_type const& >(value), type_id_with_cvr< T >());
		}
	};


	namespace impl{ namespace output{



		template < typename T, typename ... U >
		class module_output: public module_output_base{
		public:
			static constexpr auto value_types = hana::tuple_t< T, U ... >;

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_const), false, std::logical_or<>()),
				"module_output types are not allowed to be const"
			);

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_reference), false, std::logical_or<>()),
				"module_output types are not allowed to be references"
			);

	// 		static_assert(
	// 			hana::unique(hana::tuple_t< T, U ... >),
	// 			"module_output must have distict types"
	// 		);


			using module_output_base::module_output_base;


			template < typename V, typename W >
			auto put(std::size_t id, W&& value){
				static_assert(hana::contains(value_types, hana::type< V >), "type V in put< V > is not a module_output type");

				if(type_.map[hana::type< V >]){
					throw std::logic_error(
						"module_output '" + name + "' put inactive type '" + type_id_with_cvr< V >().pretty_name() + "'"
					);
				}

				return module_output_interface< V >(signal)(id, static_cast< W&& >(value));
			}


		private:
			type_t< T, U ... > type_;
		};


	} }


	template < typename T, typename ... U >
	class module_output: public impl::output::module_output< T, U ... >{
	public:
		using impl::output::module_output< T, U ... >::module_output;
	};

	template < typename T >
	class module_output< T >: public impl::output::module_output< T >{
	public:
		using impl::output::module_output< T >::module_output;

		template < typename W >
		auto put(std::size_t id, W&& value){
			impl::output::module_output< T >::template put< T >(id, static_cast< W&& >(value));
		}
	};


}


#endif
