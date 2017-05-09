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
#include "iop_list.hpp"

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


		using hana_tag = output_tag;


		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		static constexpr auto subtypes = hana::tuple_t< T ... >;

		static constexpr auto types =
			hana::transform(subtypes, TypesMetafunction{});

		static constexpr std::size_t type_count = sizeof...(T);


		using enabled_map_type = decltype(hana::make_map(
			hana::make_pair(hana::type_c< T >, false) ...));


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
			typename decltype(
				hana::unpack(types, hana::template_< std::variant >))::type
		>;


		template < typename IOP_List, typename EnabledFn >
		constexpr output(
			IOP_List const& iop_list,
			EnabledFn const& enabled_fn
		)noexcept:
			output_base(Name::value.c_str()),
			enabled_map_(hana::make_map(hana::make_pair(
				hana::type_c< T >,
				enabled_fn(iop_list, TypesMetafunction{}(hana::type_c< T >))
			) ... ))
			{}

		/// \brief Outputs are default-movable
		constexpr output(output&& other):
			output_base(std::move(other)),
			next_id_(other.next_id_),
			enabled_map_(std::move(other.enabled_map_)),
			data_(std::move(other.data_)){}


		template < typename V >
		void put(V&& value){
			static_assert(
				hana::contains(types, hana::type_c< std::decay_t< V > >),
				"type V in put< V > is not an output type"
			);

			if(!enabled_map_[hana::type_c< V >]){
				using namespace std::literals::string_literals;
				throw std::logic_error(io_tools::make_string(
					"output '", name.c_str(), "' put disabled type [",
					type_name< V >(), "]"
				));
			}

			data_.emplace(current_id(), static_cast< V&& >(value));
		}

		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(enabled_map_));
		}

		template < typename U >
		constexpr bool
		is_enabled(hana::basic_type< U > const& type)const noexcept{
			return enabled_map_[type];
		}


	protected:
		std::map< type_index, bool > enabled_types()const override{
			std::map< type_index, bool > result;
			hana::for_each(enabled_map_, [&result](auto const& x){
				auto transformed_type = TypesMetafunction{}(hana::first(x));
				result.emplace(type_index::type_id<
					typename decltype(transformed_type)::type >(),
					hana::second(x));
			});
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
						std::visit([](auto const& data)->any_type const&{
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
						}, std::move(from->second)),
						std::visit([](auto&& data)->any_type&&{
							return reinterpret_cast< any_type&& >(data);
						}, std::move(from->second)));
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

		enabled_map_type enabled_map_;

		std::multimap< std::size_t, value_type > data_;
	};


	/// \brief Provid types for constructing an output
	template <
		typename Name,
		typename OutputType,
		typename EnableFunction >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;

		/// \brief Output name as compile time string
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;

		/// \brief Type of a disposer::output
		using type = OutputType;

		/// \brief Enable function
		EnableFunction enabler;

		template < typename IOP_List >
		constexpr auto operator()(IOP_List const& iop_list)const{
			return type(iop_list, enabler);
		}
	};


	template < char ... C >
	template <
		typename Types,
		typename TypesMetafunction,
		typename EnableFunction >
	constexpr auto output_name< C ... >::operator()(
		Types const&,
		TypesMetafunction const&,
		EnableFunction&& enable_fn
	)const noexcept{
		using name_type = output_name< C ... >;
		using type_fn = std::remove_const_t< TypesMetafunction >;
		using enable_fn_t = std::remove_reference_t< EnableFunction >;

		static_assert(hana::Metafunction< TypesMetafunction >::value,
			"TypesMetafunction must model boost::hana::Metafunction");

		constexpr auto typelist = to_typelist(Types{});

		auto unpack_types =
			hana::concat(hana::tuple_t< name_type, type_fn >, typelist);

		auto type_output =
			hana::unpack(unpack_types, hana::template_< output >);

		return output_maker< name_type,
			typename decltype(type_output)::type, enable_fn_t >{
				static_cast< EnableFunction&& >(enable_fn)
			};
	}


}


#endif
