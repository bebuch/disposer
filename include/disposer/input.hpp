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
#include "input_name.hpp"
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


	template < typename Name, typename TypesMetafunction, typename ... T >
	class input: public input_base{
	public:
		static_assert(hana::is_a< input_name_tag, Name >);
		static_assert(hana::Metafunction< TypesMetafunction >::value);

		static constexpr auto subtypes = hana::make_set(hana::type_c< T > ...);

		static constexpr auto types =
			hana::make_set(TypesMetafunction{}(hana::type_c< T >) ...);

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(type_count != 0,
			"disposer::input needs at least on type");

		static_assert(!hana::any_of(subtypes, hana::traits::is_const),
			"disposer::input subtypes must not be const");

		static_assert(!hana::any_of(subtypes, hana::traits::is_reference),
			"disposer::input subtypes must not be references");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::input types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::input types must not be references");


		using value_type = std::conditional_t<
			type_count == 1,
			input_data< first_of_t< T ... > >,
			std::variant< input_data< T > ... >
		>;


		using name_type = Name;


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


	template < typename Name, typename TypesMetafunction, typename ... T >
	std::unordered_map< type_index,
		typename input< Name, TypesMetafunction, T ... >::add_function > const
		input< Name, TypesMetafunction, T ... >::type_map_ = {
			{
				type_index::type_id_with_cvr< T >(),
				&input< Name, TypesMetafunction, T ... >::add< T >
			} ...
		};


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


	template < char ... C >
	template < typename Types >
	constexpr auto
	input_name< C ... >::operator()(Types const& types)const noexcept{
		return (*this)(types, hana::template_< self_t >);
	}

	template < char ... C >
	template < typename Types, typename TypesMetafunction >
	constexpr auto input_name< C ... >::operator()(
		Types const& types,
		TypesMetafunction const&
	)const noexcept{
		using name_type = input_name< C ... >;

		static_assert(hana::Metafunction< TypesMetafunction >::value,
			"TypesMetafunction must model boost::hana::Metafunction");

		if constexpr(hana::is_a< hana::type_tag, Types >){
			using input_type =
				input< name_type, TypesMetafunction, typename Types::type >;

			return in_t< name_type, input_type >{};
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));

			auto unpack_types = hana::concat(
				hana::tuple_t< name_type, TypesMetafunction >,
				hana::to_tuple(types));

			auto type_input =
				hana::unpack(unpack_types, hana::template_< input >);

			return in_t< name_type, typename decltype(type_input)::type >{};
		}
	}

}


#endif
