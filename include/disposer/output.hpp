//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
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
#include "unpack_to.hpp"

#include <functional>


namespace disposer{


	template < typename T >
	class output_interface{
	public:
		using value_type = T;


		output_interface(signal_t& signal): signal_(signal) {}


		void operator()(std::size_t id, value_type&& value){
			exec_signal(
				id,
				std::make_shared< output_data< value_type > >(std::move(value))
			);
		}

		void operator()(std::size_t id, value_type const& value){
			exec_signal(
				id,
				std::make_shared< output_data< value_type > >(value)
			);
		}

		void operator()(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			exec_signal(id, value);
		}


	private:
		signal_t& signal_;

		void exec_signal(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			signal_(
				id,
				reinterpret_cast< any_type const& >(value),
				type_index::type_id_with_cvr< T >()
			);
		}
	};


	template < typename ... T >
	class basic_output: public output_base{
	public:
		static constexpr auto types = hana::make_set(hana::type_c< T > ...);

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(type_count != 0,
			"disposer::output needs at least on type");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::output types are not allowed to be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::output types are not allowed to be references");


		using output_base::output_base;


		template < typename V, typename W >
		void put(W&& value){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in put< V > is not a output type"
			);

			if(!enabled_types_[type_index::type_id_with_cvr< V >()]){
				throw std::logic_error(
					"output '" + name + "' put disabled type [" +
					type_name_with_cvr< V >() + "]"
				);
			}

			output_interface< V >{signal}(id, static_cast< W&& >(value));
		}


		template < typename V >
		void enable(){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in enable< V > is not a output type"
			);

			enabled_types_[type_index::type_id_with_cvr< V >()] = true;
		}

		template < typename V, typename W, typename ... X >
		void enable(){
			enable< V >();
			enable< W, X ... >();
		}

		void enable_types(std::vector< type_index > const& types){
			for(auto& type: types){
				auto iter = std::find(
					enabled_types_.begin(),
					enabled_types_.end(),
					type
				);

				if(iter == enabled_types_.end()){
					throw std::logic_error(
						"type [" + type.pretty_name() +
						"] is not an output type of '" + name + "'"
					);
				}

				iter->second = true;
			}
		}


	protected:
		virtual std::vector< type_index > enabled_types()const override{
			std::vector< type_index > result;
			result.reserve(type_count);
			for(auto const& [type, enabled]: enabled_types_){
				if(enabled) result.push_back(type);
			}
			return result;
		}


	private:
		std::unordered_map< type_index, bool > enabled_types_{
				{type_index::type_id_with_cvr< T >(), false} ...
			};
	};


	template < typename ... T >
	struct output: basic_output< T ... >{
		using basic_output< T ... >::basic_output;
	};


	template < typename T >
	struct output< T >: basic_output< T >{
		using basic_output< T >::basic_output;

		template < typename W >
		void put(W&& value){
			basic_output< T >::template put< T >(
				static_cast< W&& >(value)
			);
		}
	};


	template <
		template < typename > typename Container,
		typename Set,
		typename = hana::when< true > >
	struct container_output;

	template <
		template < typename > typename Container,
		typename Set >
	struct container_output<
		Container,
		Set,
		hana::when< hana::is_a< hana::set_tag, Set > >
	>: decltype(unpack_with_container_to< Container, output >(Set{}))::type{
		using base_class = typename
			decltype(unpack_with_container_to< Container, output >(Set{}))
				::type;

		using base_class::output;

		template < typename ... V >
		void enable(){
			base_class::template enable< Container< V > ... >();
		}

		template < typename V, typename W >
		void put_by_subtype(W&& value){
			base_class::template
				put< Container< V > >(static_cast< W&& >(value));
		}
	};


}


#endif
