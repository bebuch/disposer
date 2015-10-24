//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__output__hpp_INCLUDED_
#define _disposer__output__hpp_INCLUDED_

#include "container_lists.hpp"
#include "input_base.hpp"
#include "output_base.hpp"
#include "output_data.hpp"
#include "is_type_unique.hpp"
#include "type_position.hpp"

#include <boost/hana.hpp>

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
			trigger_signal(
				id,
				std::make_shared< output_data< value_type > >(std::move(value))
			);
		}

		void operator()(std::size_t id, value_type const& value){
			trigger_signal(
				id,
				std::make_shared< output_data< value_type > >(value)
			);
		}

		void operator()(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			trigger_signal(id, value);
		}


	private:
		signal_t& signal_;

		void trigger_signal(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			signal_(
				id,
				reinterpret_cast< any_type const& >(value),
				type_id_with_cvr< T >()
			);
		}
	};


	namespace impl{ namespace output{


		template < typename T, typename ... U >
		class output: public output_base{
		public:
			static constexpr auto value_types = hana::tuple_t< T, U ... >;

			static_assert(
				!hana::fold(hana::transform(
					value_types,
					hana::traits::is_const
				), false, std::logical_or<>()),
				"disposer::output types are not allowed to be const"
			);

			static_assert(
				!hana::fold(hana::transform(
					value_types,
					hana::traits::is_reference
				), false, std::logical_or<>()),
				"disposer::output types are not allowed to be references"
			);

			static_assert(
				is_type_unique< T, U ... >,
				 "disposer::output must have distict types"
			);


			using output_base::output_base;


			template < typename V, typename W >
			auto put(W&& value){
				static_assert(
					hana::contains(value_types, hana::type_c< V >),
					"type V in put< V > is not a output type"
				);

				if(!active_types_[type_position_v< V, T, U ... >]){
					throw std::logic_error(
						"output '" + name + "' put inactive type '" +
						type_id_with_cvr< V >().pretty_name() + "'"
					);
				}

				return output_interface< V >(signal)(
					id, static_cast< W&& >(value)
				);
			}


			template < typename V >
			void activate(){
				static_assert(
					hana::contains(value_types, hana::type_c< V >),
					"type V in activate< V > is not a output type"
				);

				active_types_[type_position_v< V, T, U ... >] = true;
			}

			template < typename V, typename W, typename ... X >
			void activate(){
				activate< V >();
				activate< W, X ... >();
			}

			void activate_types(std::vector< type_index > const& types){
				for(auto& type: types){
					auto iter = std::find(
						type_indices_.begin(),
						type_indices_.end(),
						type
					);

					if(iter == type_indices_.end()){
						throw std::runtime_error(
							"type '" + type.pretty_name() +
							"' is not an output type of '" + name + "'"
						);
					}

					auto index = iter - type_indices_.begin();
					active_types_[index] = true;
				}
			}


		protected:
			virtual std::vector< type_index > active_types()const override{
				std::vector< type_index > result;
				result.reserve(1 + sizeof...(U));
				for(std::size_t i = 0; i < 1 + sizeof...(U); ++i){
					if(active_types_[i]) result.push_back(type_indices_[i]);
				}
				return result;
			}


		private:
			static std::array< type_index, 1 + sizeof...(U) > const
				type_indices_;

			std::array< bool, 1 + sizeof...(U) > active_types_{{false}};
		};

		template < typename T, typename ... U >
		std::array< type_index, 1 + sizeof...(U) > const
		output< T, U ... >::type_indices_{{
			type_id_with_cvr< T >(),
			type_id_with_cvr< U >() ...
		}};


		template < 
			template < typename, typename ... > class Container,
			typename ... T
		>
		class container_output: public output< Container< T > ... >{
		public:
			using output< Container< T > ... >::output;


			template < typename V >
			void activate(){
				output< Container< T > ... >::
				template activate< Container< V > >();
			}

			template < typename V, typename W, typename ... X >
			void activate(){
				output< Container< T > ... >::
				template activate<
					Container< V >, Container< W >, Container< X > ...
				>();
			}
		};


	} }


	template < typename T, typename ... U >
	struct output: impl::output::output< T, U ... >{
		using impl::output::output< T, U ... >::output;
	};

	template < typename T >
	struct output< T >: impl::output::output< T >{
		using impl::output::output< T >::output;

		template < typename W >
		auto put(W&& value){
			impl::output::output< T >::template put< T >(
				static_cast< W&& >(value)
			);
		}
	};

	template < typename T, typename ... U >
	struct output< type_list< T, U ... > >: output< T, U ... >{
		using output< T, U ... >::output;
	};



	template <
		template< typename, typename ... > class Container, typename ... T
	>
	struct container_output:
		impl::output::container_output< Container, T ... >
	{
	public:
		using impl::output::
			container_output< Container, T ... >::container_output;

		template < typename V, typename W >
		auto put(W&& value){
			impl::output::container_output< Container, T ... >::
				template put< Container< V > >(static_cast< W&& >(value));
		}
	};

	template < template< typename, typename ... > class Container, typename T >
	struct container_output< Container, T >:
		impl::output::container_output< Container, T >
	{
		using impl::output::container_output< Container, T >::container_output;

		template < typename W >
		auto put(W&& value){
			impl::output::container_output< Container, T >::
				template put< Container< T > >(static_cast< W&& >(value));
		}
	};

	template <
		template< typename, typename ... > class Container,
		typename ... T
	>
	struct container_output< Container, type_list< T ... > >:
		container_output< Container, T ... >
	{
		using container_output< Container, T ... >::container_output;
	};



}


#endif
