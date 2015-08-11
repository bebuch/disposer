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
#include "is_type_unique.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/type_traits.hpp>

#include <functional>


namespace disposer{


	namespace hana = boost::hana;

	using boost::typeindex::type_id_with_cvr;


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


		template < typename Ref, std::size_t Index, typename ... Tests >
		struct type_position;

		template < std::size_t Index, bool valid >
		struct type_position_test: std::integral_constant< std::size_t, Index >{};

		template < std::size_t Index >
		struct type_position_test< Index, false >{
			static_assert(Index == -1, "type_position did not find the type");
		};

		template < typename Ref, std::size_t Index, typename Test >
		struct type_position< Ref, Index, Test >: type_position_test< Index, std::is_same< Ref, Test >::value >{};

		template < typename Ref, std::size_t Index, typename Test, typename ... Tests >
		struct type_position< Ref, Index, Test, Tests ... >: std::conditional_t<
			std::is_same< Ref, Test >::value,
			std::integral_constant< std::size_t, Index >,
			type_position< Ref, 1 + Index, Tests ... >
		>{};

		template < typename Ref, typename ... Tests >
		constexpr std::size_t type_position_v = type_position< Ref, 0, Tests ... >::value;


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

			static_assert(is_type_unique< T, U ... >, "output must have distict types");


			using output_base::output_base;


			virtual std::vector< type_index > active_types()const override{
				std::vector< type_index > result;
				result.reserve(1 + sizeof...(U));
				for(std::size_t i = 0; i < 1 + sizeof...(U); ++i){
					if(active_types_[i]) result.push_back(type_indices_[i]);
				}
				return result;
			}


			template < typename V, typename W >
			auto put(std::size_t id, W&& value){
				static_assert(hana::contains(value_types, hana::type< V >), "type V in put< V > is not a output type");

				if(!active_types_[type_position_v< V, T, U ... >]){
					throw std::logic_error(
						"output '" + name + "' put inactive type '" + type_id_with_cvr< V >().pretty_name() + "'"
					);
				}

				return output_interface< V >(signal)(id, static_cast< W&& >(value));
			}

			template < typename V >
			void activate(){
				static_assert(hana::contains(value_types, hana::type< V >), "type V in activate< V > is not a output type");

				active_types_[type_position_v< V, T, U ... >] = true;
			}

			template < typename V, typename W, typename ... X >
			void activate(){
				activate< V >();
				activate< W, X ... >();
			}


		private:
			static std::array< type_index, 1 + sizeof...(U) > const type_indices_;

			std::array< bool, 1 + sizeof...(U) > active_types_{{false}};
		};

		template < typename T, typename ... U >
		std::array< type_index, 1 + sizeof...(U) > const output< T, U ... >::type_indices_{{ type_id_with_cvr< T >(), type_id_with_cvr< U >() ... }};


		template < template< typename, typename ... > class Container, typename ... T >
		class container_output: public output< Container< T > ... >{
		public:
			using output< Container< T > ... >::output;

			template < typename V >
			void activate(){
				output< Container< T > ... >::template activate< Container< V > >();
			}

			template < typename V, typename W, typename ... X >
			void activate(){
				output< Container< T > ... >::template activate< Container< V >, Container< W >, Container< X > ... >();
			}
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


	template < template< typename, typename ... > class Container, typename T, typename ... U >
	class container_output: public impl::output::container_output< Container, T, U ... >{
	public:
		using impl::output::container_output< Container, T, U ... >::container_output;

		template < typename V, typename W >
		auto put(std::size_t id, W&& value){
			impl::output::container_output< Container, T, U ... >::template put< Container< V > >(id, static_cast< W&& >(value));
		}
	};

	template < template< typename, typename ... > class Container, typename T >
	class container_output< Container, T >: public impl::output::container_output< Container, T >{
	public:
		using impl::output::container_output< Container, T >::container_output;

		template < typename W >
		auto put(std::size_t id, W&& value){
			impl::output::container_output< Container, T >::template put< Container< T > >(id, static_cast< W&& >(value));
		}
	};



}


#endif
