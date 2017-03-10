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
#include "io.hpp"

#include <io_tools/make_string.hpp>

#include <variant>
#include <set>
#include <map>


namespace disposer{


	template < typename T, typename ... >
	struct first_of{
		using type = T;
	};

	template < typename ... T >
	using first_of_t = typename first_of< T ... >::type;


	template < typename Name, typename ... T >
	class input: public input_base{
	public:
		static_assert(hana::is_a< hana::string_tag, Name >);

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
			input_data< first_of_t< T ... > >,
			std::variant< input_data< T > ... >
		>;


		using input_base::input_base;

		constexpr input()noexcept: input_base(Name::c_str()) {}


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
				throw std::logic_error(io_tools::make_string(
					"unknown add type [", type.pretty_name(),
					"] in input '", name, "'"
				));
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


	template < typename Name, typename ... T >
	std::unordered_map< type_index, typename input< Name, T ... >
		::add_function > const input< Name, T ... >::type_map_ = {
			{
				type_index::type_id_with_cvr< T >(),
				&input< Name, T ... >::add< T >
			} ...
		};


}


namespace disposer::interface::module{


	/// \brief Provid types for constructing an input
	template < typename Name, typename InputType >
	struct in_t: io< in_t< Name, InputType > >{
		/// \brief Tag for boost::hana
		using hana_tag = in_tag;

		/// \brief Output name as compile time string
		using name = Name;

		/// \brief Type of a disposer::input
		using type = InputType;
	};

	template < typename Name, typename Types >
	constexpr auto in(Name&&, Types&&){
		using raw_name = std::remove_cv_t< std::remove_reference_t< Name > >;
		using raw_types = std::remove_cv_t< std::remove_reference_t< Types > >;

		static_assert(hana::is_a< hana::string_tag, raw_name >,
			"Name must be of type boost::hana::string< ... >");

		if constexpr(hana::is_a< hana::type_tag, raw_types >){
			using input_type =
				::disposer::input< raw_name, typename raw_types::type >;

			return in_t< raw_name, input_type >{};
		}else{
			static_assert(hana::Foldable< raw_types >::value);
			static_assert(hana::all_of(raw_types{},
				hana::is_a< hana::type_tag >));

			constexpr auto string_and_types = hana::prepend(
				hana::to_tuple(raw_types{}), hana::type_c< Name >);

			constexpr auto type_input =
				::disposer::unpack_to< ::disposer::input >(string_and_types);

			return in_t< raw_name, typename decltype(+type_input)::type >{};
		}
	}


}


#endif
