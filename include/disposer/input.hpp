//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__input__hpp_INCLUDED_
#define _disposer__input__hpp_INCLUDED_

#include "container_lists.hpp"
#include "input_base.hpp"
#include "input_data.hpp"
#include "are_types_distinct.hpp"

#include <variant>
#include <set>
#include <map>


namespace disposer{


	using boost::typeindex::type_id_with_cvr;


	template < typename T, typename ... U >
	class input: public input_base{
	public:
		static constexpr auto value_types = hana::tuple_t< T, U ... >;

		using value_type = std::conditional_t<
			sizeof...(U) == 0,
			input_data< T >,
			std::variant< input_data< T >, input_data< U > ... >
		>;

		static_assert(
			!hana::fold(hana::transform(
				value_types, hana::traits::is_const
			), false, std::logical_or<>()),
			"disposer::input types are not allowed to be const"
		);

		static_assert(
			!hana::fold(hana::transform(
				value_types, hana::traits::is_reference
			), false, std::logical_or<>()),
			"disposer::input types are not allowed to be references"
		);

		static_assert(
			are_types_distinct_v< T, U ... >,
			"disposer::input must have distict types"
		);


		using input_base::input_base;


		std::multimap< std::size_t, value_type > get(){
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);

			std::multimap< std::size_t, value_type > result(
				std::make_move_iterator(from), std::make_move_iterator(to)
			);
			data_.erase(from, to);

			return result;
		}

		std::vector< type_index > enabled_types()const{
			std::vector< type_index > result;
			result.reserve(1 + sizeof...(U));
			for(auto& type: enabled_map_){
				if(type.second) result.push_back(type.first);
			}
			return result;
		}


		template < typename TransformFunction >
		std::vector< type_index > enabled_types_transformed(
// 			hana::basic_type< Out >(*fn)(hana::basic_type< In >)
			TransformFunction fn
		)const{
			std::map< type_index, type_index > transform_map = {
				{ type_id_with_cvr< T >(), type_id_with_cvr
					< typename decltype(fn(hana::type_c< T >))::type >() },
				{ type_id_with_cvr< U >(), type_id_with_cvr
					< typename decltype(fn(hana::type_c< U >))::type >() } ...
			};

			std::vector< type_index > result;
			result.reserve(1 + sizeof...(U));
			for(auto& type: enabled_map_){
				if(!type.second) continue;
				result.push_back(transform_map.at(type.first));
			}
			return result;
		}


	private:
		virtual void add(
			std::size_t id,
			any_type const& value,
			type_index const& type,
			bool last_use
		)override{
			auto iter = type_map_.find(type);
			if(iter == type_map_.end()){
				throw std::logic_error(
					"unknown add type '" + type.pretty_name() +
					"' in input + '" + name + "'"
				);
			}

			// Call add< type >(id, value, last_use)
			(this->*(iter->second))(id, value, last_use);
		}


		virtual void cleanup(std::size_t id)noexcept override{
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);
			data_.erase(from, to);
		}

		virtual std::vector< type_index > types()const override{
			std::vector< type_index > result;
			result.reserve(enabled_map_.size());
			for(auto& pair: enabled_map_){
				result.push_back(pair.first);
			}
			return result;
		}

		virtual bool enable_types(
			std::vector< type_index > const& types
		)noexcept override{
			for(auto& type: types){
				auto iter = enabled_map_.find(type);

				if(iter == enabled_map_.end()) return false;

				iter->second = true;
			}

			return true;
		}


		template < typename V >
		void add(std::size_t id, any_type const& value, bool last_use){
			auto data = reinterpret_cast< output_data_ptr< V > const& >(value);

			std::lock_guard< std::mutex > lock(mutex_);
			data_.emplace(id, input_data< V >(data, last_use));
		}


		static std::map<
			type_index, void(input::*)(std::size_t, any_type const&, bool)
		> const type_map_;

		std::map< type_index, bool > enabled_map_ = {
			{ type_id_with_cvr< T >(), false },
			{ type_id_with_cvr< U >(), false } ...
		};

		std::mutex mutex_;

		std::multimap< std::size_t, value_type > data_;
	};

	template < typename T, typename ... U >
	class input< type_list< T, U ... > >: public input< T, U ... >{};

	template < typename T, typename ... U >
	std::map<
		type_index,
		void(input< T, U ... >::*)(std::size_t, any_type const&, bool)
	> const input< T, U ... >::type_map_ = {
			{ type_id_with_cvr< T >(), &input< T, U ... >::add< T > },
			{ type_id_with_cvr< U >(), &input< T, U ... >::add< U > } ...
		};

	template <
		template< typename, typename ... > class Container, typename ... T
	> struct container_input:
		input< Container< T > ... >
	{
		using input< Container< T > ... >::input;
	};

	template <
		template< typename, typename ... > class Container, typename ... T
	> struct container_input< Container, type_list< T ... > >:
		container_input< Container, T ... >
	{
		using container_input< Container, T ... >::container_input;
	};


}


#endif
