//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_output_hpp_INCLUDED_
#define _disposer_output_hpp_INCLUDED_

#include "input_base.hpp"
#include "output_base.hpp"
#include "output_data.hpp"

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
	class output_interface{
	public:
		using value_type = T;


		output_interface(signal_t& signal): signal_(signal) {}


		void operator()(std::size_t id, value_type&& value){
			trigger_signal(id, std::make_shared< output_data< value_type > >(std::move(value)));
		}

		void operator()(std::size_t id, value_type const& value){
			trigger_signal(id, std::make_shared< output_data< value_type > >(value));
		}

		void operator()(std::size_t id, output_data_ptr< value_type > const& value){
			trigger_signal(id, value);
		}


	private:
		signal_t& signal_;

		void trigger_signal(std::size_t id, output_data_ptr< value_type > const& value){
			signal_(id, reinterpret_cast< any_type const& >(value), type_id_with_cvr< T >());
		}
	};


	namespace impl{ namespace output{



		template < typename T, typename ... U >
		class output: public output_base{
		public:
			static constexpr auto value_types = hana::tuple_t< T, U ... >;

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_const), false, std::logical_or<>()),
				"output types are not allowed to be const"
			);

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_reference), false, std::logical_or<>()),
				"output types are not allowed to be references"
			);

	// 		static_assert(
	// 			hana::unique(hana::tuple_t< T, U ... >),
	// 			"output must have distict types"
	// 		);


			using output_base::output_base;


			template < typename V, typename W >
			auto put(std::size_t id, W&& value){
				static_assert(hana::contains(value_types, hana::type< V >), "type V in put< V > is not a output type");

				if(type_.map[hana::type< V >]){
					throw std::logic_error(
						"output '" + name + "' put inactive type '" + type_id_with_cvr< V >().pretty_name() + "'"
					);
				}

				return output_interface< V >(signal)(id, static_cast< W&& >(value));
			}


		private:
			type_t< T, U ... > type_;
		};


	} }


	template < typename T, typename ... U >
	class output: public impl::output::output< T, U ... >{
	public:
		using impl::output::output< T, U ... >::output;
	};

	template < typename T >
	class output< T >: public impl::output::output< T >{
	public:
		using impl::output::output< T >::output;

		template < typename W >
		auto put(std::size_t id, W&& value){
			impl::output::output< T >::template put< T >(id, static_cast< W&& >(value));
		}
	};


}


#endif
