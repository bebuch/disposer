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

#include "output_base.hpp"
#include "type_index.hpp"
#include "output_name.hpp"
#include "module_config_lists.hpp"

#include <io_tools/make_string.hpp>

#include <functional>
#include <variant>
#include <mutex>


namespace disposer{


	template < typename Name, typename TypesMetafunction, typename ... T >
	class output: public output_base{
	public:
		static_assert(hana::is_a< output_name_tag, Name >);
		static_assert(hana::Metafunction< TypesMetafunction >::value);


		using name_type = Name;


		static constexpr auto subtypes = hana::tuple_t< T ... >;

		static constexpr auto types =
			hana::transform(subtypes, TypesMetafunction{});

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(hana::length(subtypes) ==
			hana::length(hana::to_set(subtypes)),
			"disposer::output needs all subtypes T to be distinct");

		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::output needs all types T to be distinct");

		static_assert(type_count != 0,
			"disposer::output needs at least on type");

		static_assert(!hana::any_of(subtypes, hana::traits::is_const),
			"disposer::output subtypes must not be const");

		static_assert(!hana::any_of(subtypes, hana::traits::is_reference),
			"disposer::output subtypes must not be references");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::output types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::output types must not be references");


		using value_type = std::conditional_t<
			type_count == 1,
			typename decltype(+types[hana::int_c< 0 >])::type,
			decltype(hana::unpack(types, hana::template_< std::variant >))
		>;


		output(): output_base(Name::value.c_str()) {}


		template < typename V >
		void put(V&& value){
			static_assert(
				hana::contains(types, hana::type_c< std::decay_t< V > >),
				"type V in put< V > is not an output type"
			);

			if(!enabled_types_[type_index::type_id< V >()]){
				using namespace std::literals::string_literals;
				throw std::logic_error(io_tools::make_string(
					"output '", name, "' put disabled type [",
					type_name< V >(), "]"
				));
			}

			data_.emplace(current_id(), static_cast< V&& >(value));
		}


		template < typename V >
		void enable(){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in enable< V > is not an output type"
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
					throw std::logic_error(io_tools::make_string(
						"type [", type.pretty_name(),
						"] is not an output type of '", name, "'"
					));
				}

				iter->second = true;
			}
		}


	protected:
		std::vector< type_index > enabled_types()const override{
			std::vector< type_index > result;
			result.reserve(type_count);
			for(auto const& pair: enabled_types_){
				if(pair.second) result.push_back(pair.first);
			}
			return result;
		}


	private:
		virtual std::vector< reference_carrier >
		get_references(std::size_t id)override{
			std::lock_guard< std::mutex > lock(mutex_);

			assert(id >= next_id_);

			auto from = data_.lower_bound(next_id_);
			auto const to = data_.upper_bound(id);

			std::vector< reference_carrier > result;
			result.reserve(std::distance(from, to));

			for(; from != to; ++from){
				if constexpr(type_count == 1){
					result.emplace_back(
						from->first,
						type_index::type_id< decltype(from->second) >(),
						reinterpret_cast< any_type const& >(from->second));
				}else{
					result.emplace_back(
						from->first,
						std::visit([](auto const& data){
							return type_index::type_id< decltype(data) >();
						}, from->second),
						std::visit([](auto const& data){
							return reinterpret_cast< any_type const& >(data);
						}, from->second));
				}
			}

			next_id_ = id + 1;

			return result;
		}

		virtual void transfer_values(
			std::size_t id,
			TransferFn const& fn
		)override{
			std::lock_guard< std::mutex > lock(mutex_);

			assert(id >= next_id_);

			auto from = data_.lower_bound(next_id_);
			auto const to = data_.upper_bound(id);

			std::vector< value_carrier > result;
			result.reserve(std::distance(from, to));

			for(; from != to; ++from){
				if constexpr(type_count == 1){
					result.emplace_back(
						from->first,
						type_index::type_id< decltype(from->second) >(),
						reinterpret_cast< any_type&& >(from->second));
				}else{
					result.emplace_back(
						from->first,
						std::visit([](auto&& data){
							return type_index::type_id< decltype(data) >();
						}, from->second),
						std::visit([](auto&& data){
							return reinterpret_cast< any_type&& >(data);
						}, from->second));
				}
			}

			fn(std::move(result));

			remove_until(id);
		}

		virtual void cleanup(std::size_t id)noexcept override{
			std::lock_guard< std::mutex > lock(mutex_);

			assert(id >= next_id_);

			remove_until(id);
		}

		void remove_until(std::size_t id)noexcept{
			auto from = data_.begin();
			auto to = data_.upper_bound(id);
			data_.erase(from, to);

			if(next_id_ < id + 1) next_id_ = id + 1;
		}



		std::mutex mutex_;

		std::size_t next_id_{0};

		std::unordered_map< type_index, bool > enabled_types_{
				{type_index::type_id_with_cvr< T >(), false} ...
			};

		std::multimap< std::size_t, value_type > data_;
	};


	/// \brief Provid types for constructing an output
	template < typename Name, typename OutputType >
	struct out_t{
		/// \brief Tag for boost::hana
		using hana_tag = out_tag;

		/// \brief Output name as compile time string
		using name_type = Name;

		/// \brief Type of a disposer::output
		using type = OutputType;
	};


	template < char ... C >
	template < typename Types >
	constexpr auto
	output_name< C ... >::operator()(Types const& types)const noexcept{
		return (*this)(types, hana::template_< self_t >);
	}

	template < char ... C >
	template < typename Types, typename TypesMetafunction >
	constexpr auto output_name< C ... >::operator()(
		Types const& types,
		TypesMetafunction const&
	)const noexcept{
		using name_type = output_name< C ... >;

		static_assert(hana::Metafunction< TypesMetafunction >::value,
			"TypesMetafunction must model boost::hana::Metafunction");

		if constexpr(hana::is_a< hana::type_tag, Types >){
			using output_type =
				output< name_type, TypesMetafunction, typename Types::type >;

			return out_t< name_type, output_type >{};
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));

			auto unpack_types = hana::concat(
				hana::tuple_t< name_type, TypesMetafunction >,
				hana::to_tuple(types));

			auto type_output =
				hana::unpack(unpack_types, hana::template_< output >);

			return out_t< name_type, typename decltype(type_output)::type >{};
		}
	}


}


#endif
