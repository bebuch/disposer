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

#include "input_base.hpp"
#include "input_data.hpp"
#include "unpack_to.hpp"
#include "type_index.hpp"

#include <io_tools/make_string.hpp>

#include <variant>
#include <set>
#include <map>


namespace disposer{


	template < typename ... T >
	class input: public input_base{
	public:
		static constexpr auto types = hana::make_set(hana::type_c< T > ...);

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(type_count != 0,
			"disposer::input needs at least on type");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::input types are not allowed to be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::input types are not allowed to be references");


		using value_type = std::conditional_t<
			type_count == 1,
			input_data< T ... >,
			std::variant< input_data< T > ... >
		>;


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
			result.reserve(type_count);
			for(auto& type: enabled_map_){
				if(type.second) result.push_back(type.first);
			}
			return result;
		}


		template < typename TransformFunction >
		std::vector< type_index > enabled_types_transformed(
			// hana::basic_type< Out >(*fn)(hana::basic_type< In >)
			TransformFunction fn
		)const{
			std::unordered_map< type_index, type_index > transform_map = {
				{
					type_index::type_id_with_cvr< T >(),
					type_index::type_id_with_cvr<
						typename decltype(fn(hana::type_c< T >))::type >()
				} ...
			};

			std::vector< type_index > result;
			result.reserve(type_count);
			for(auto& type: enabled_map_){
				if(!type.second) continue;
				result.push_back(transform_map.at(type.first));
			}
			return result;
		}


	private:
		using add_function = void(input::*)(std::size_t, any_type const&, bool);


		virtual void add(
			std::size_t id,
			any_type const& value,
			type_index const& type,
			bool last_use
		)override{
			auto iter = type_map_.find(type);
			if(iter == type_map_.end()){
				throw std::logic_error(
					"unknown add type [" + type.pretty_name() +
					"] in input '" + name + "'"
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

		virtual std::vector< type_index > type_list()const override{
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


		static std::unordered_map< type_index, add_function > const type_map_;

		std::unordered_map< type_index, bool > enabled_map_ = {
			{ type_index::type_id_with_cvr< T >(), false } ...
		};

		std::mutex mutex_;

		std::multimap< std::size_t, value_type > data_;
	};

	template < typename ... T >
	std::unordered_map< type_index, typename input< T ... >::add_function >
		const input< T ... >::type_map_ = {
			{
				type_index::type_id_with_cvr< T >(),
				&input< T ... >::add< T >
			} ...
		};


// 	template <
// 		template < typename > typename Container,
// 		typename Set,
// 		typename = hana::when< true > >
// 	struct container_input;
//
// 	template <
// 		template < typename > typename Container,
// 		typename Set >
// 	struct container_input<
// 		Container,
// 		Set,
// 		hana::when< hana::is_a< hana::set_tag, Set > >
// 	>: decltype(unpack_with_container_to< Container, input >(Set{}))::type{
// 		using base_class = typename
// 			decltype(unpack_with_container_to< Container, input >(Set{}))
// 				::type;
//
// 		using base_class::input;
// 	};


}


#endif
