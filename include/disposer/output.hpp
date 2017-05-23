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


	template < typename Name, typename TypeTransformFn, typename ... T >
	class output: public output_base{
	public:
		static_assert(hana::is_a< output_name_tag, Name >);


		using hana_tag = output_tag;


		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		static constexpr auto type_transform =
			type_transform_fn< TypeTransformFn >{};


		static constexpr auto subtypes = hana::tuple_t< T ... >;

		static constexpr auto types =
			hana::transform(subtypes, type_transform);

		static constexpr std::size_t type_count = sizeof...(T);


		using enabled_map_type = decltype(hana::make_map(
			hana::make_pair(type_transform(hana::type_c< T >), false) ...));


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


		constexpr output(enabled_map_type&& enable_map)noexcept:
			output_base(Name::value.c_str()),
			enabled_map_(std::move(enable_map))
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
				hana::contains(types, hana::type_c< V >),
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
			auto const is_type_valid = hana::contains(enabled_map_, type);
			static_assert(is_type_valid, "type in not an input type");
			return enabled_map_[type];
		}

		template < typename U >
		constexpr bool
		is_subtype_enabled(hana::basic_type< U > const& type)const noexcept{
			return is_enabled(type_transform(type));
		}


	protected:
		std::map< type_index, bool > enabled_types()const override{
			std::map< type_index, bool > result;
			hana::for_each(enabled_map_, [&result](auto const& x){
				auto transformed_type = hana::first(x);
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
		typename EnableFn >
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
		enable_fn< EnableFn > enable;

		template < typename IOP_List >
		constexpr auto operator()(IOP_List const& iop_list)const{
			return type(hana::unpack(hana::transform(type::subtypes,
				[&](auto subtype){
					return hana::make_pair(type::type_transform(subtype),
						enable(iop_list, subtype));
				}), hana::make_map));
		}
	};


	template <
		typename Name,
		typename Types,
		typename TypeTransformFn,
		typename EnableFn >
	constexpr auto create_output_maker(
		Name const&,
		Types const&,
		type_transform_fn< TypeTransformFn >&&,
		enable_fn< EnableFn >&& enable
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name, TypeTransformFn >, typelist);

		constexpr auto type_output =
			hana::unpack(unpack_types, hana::template_< output >);

		return output_maker< Name,
			typename decltype(type_output)::type, EnableFn >{
				std::move(enable)
			};
	}


	template < char ... C >
	template <
		typename Types,
		typename Arg2,
		typename Arg3 >
	constexpr auto output_name< C ... >::operator()(
		Types const& types,
		Arg2&& arg2,
		Arg3&& arg3
	)const{
		constexpr auto valid_argument = [](auto const& arg){
				return hana::is_a< type_transform_fn_tag >(arg)
					|| hana::is_a< enable_fn_tag >(arg)
					|| hana::is_a< no_argument_tag >(arg);
			};

		auto const arg2_valid = valid_argument(arg2);
		static_assert(arg2_valid, "argument 2 is invalid");
		auto const arg3_valid = valid_argument(arg3);
		static_assert(arg3_valid, "argument 3 is invalid");

		auto args = hana::make_tuple(
			static_cast< Arg2&& >(arg2),
			static_cast< Arg3&& >(arg3)
		);

		auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(tt, "more than one type_transform_fn");
		auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(ef, "more than one enable_fn");

		return create_output_maker(
			(*this),
			types,
			get_or_default(std::move(args),
				hana::is_a< type_transform_fn_tag >,
				type_transform_fn< no_transform >{}),
			get_or_default(std::move(args),
				hana::is_a< enable_fn_tag >,
				enable_fn< enable_always >{})
		);
	}


}


#endif
