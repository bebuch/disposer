//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_module_input_hpp_INCLUDED_
#define _disposer_module_input_hpp_INCLUDED_

#include "module_input_base.hpp"
#include "module_input_data.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/type_traits.hpp>
#include <boost/variant.hpp>

#include <functional>
#include <map>


namespace disposer{


	namespace hana = boost::hana;

	using boost::typeindex::type_id_with_cvr;


	template < typename T, typename ... U >
	class module_input: public module_input_base{
	public:
		static constexpr auto value_types = hana::tuple_t< T, U ... >;

		using value_type = std::conditional_t<
			sizeof...(U) == 0,
			module_input_data< T >,
			boost::variant< module_input_data< T >, module_input_data< U > ... >
		>;

		static_assert(
			!hana::fold(hana::transform(value_types, hana::traits::is_const), false, std::logical_or<>()),
			"module_input types are not allowed to be const"
		);

		static_assert(
			!hana::fold(hana::transform(value_types, hana::traits::is_reference), false, std::logical_or<>()),
			"module_input types are not allowed to be references"
		);

		// TODO: distict types


		using module_input_base::module_input_base;


		virtual void add(std::size_t id, any_type const& value, type_index const& type, bool last_use)override{
			auto iter = type_map_.find(type);
			if(iter == type_map_.end()){
				throw std::logic_error("unknown add type '" + type.pretty_name() + "' in module_input + '" + name + "'");
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


		std::multimap< std::size_t, value_type > get(std::size_t id){
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);

			std::multimap< std::size_t, value_type > result(std::make_move_iterator(from), std::make_move_iterator(to));
			data_.erase(from, to);

			return result;
		}

		bool does_accept(std::vector< type_index > const& types)const noexcept override{
			static auto const input_types = hana::make_tuple(type_id_with_cvr< T >(), type_id_with_cvr< U >() ...);

			for(auto& type: types){
				if(!hana::contains(input_types, type)) return false;
			}

			return true;
		}


	private:
		template < typename V >
		void add(std::size_t id, any_type const& value, bool last_use){
			auto data = reinterpret_cast< module_output_data_ptr< V > const& >(value);

			std::lock_guard< std::mutex > lock(mutex_);
			data_.emplace(id, module_input_data< V >(data, last_use));
		}


		static std::map< type_index, void(module_input::*)(std::size_t, any_type const&, bool) > const type_map_;

		std::mutex mutex_;

		std::multimap< std::size_t, value_type > data_;
	};

	template < typename T, typename ... U >
	std::map< type_index, void(module_input< T, U ... >::*)(std::size_t, any_type const&, bool) > const module_input< T, U ... >::type_map_ = {
			{ type_id_with_cvr< T >(), &module_input< T, U ... >::add< T > },
			{ type_id_with_cvr< U >(), &module_input< T, U ... >::add< U > } ...
		};


}


#endif
