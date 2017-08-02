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

#include "detail/output_base.hpp"

#include "config_fn.hpp"

#include "tool/type_index.hpp"
#include "tool/to_std_string_view.hpp"

#include <io_tools/make_string.hpp>

#include <functional>
#include <variant>


namespace disposer{


	/// \brief Hana Tag for \ref output_name
	struct output_name_tag{};

	/// \brief Hana Tag for output_maker
	struct output_maker_tag{};

	/// \brief Hana Tag for output
	struct output_tag{};

	template < typename Name, typename TypeTransformFn, typename ... T >
	class output: public output_base{
	public:
		static_assert(hana::is_a< output_name_tag, Name >);


		/// \brief Hana tag to identify outputs
		using hana_tag = output_tag;


		/// \brief Compile time name of the output
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		/// \brief Meta function to transfrom subtypes to the actual types
		static constexpr auto type_transform =
			type_transform_fn< TypeTransformFn >{};

		/// \brief Subtypes (before type_transform) as hana::tuple
		static constexpr auto subtypes = hana::tuple_t< T ... >;

		/// \brief Types (after type_transform) as hana::tuple
		static constexpr auto types =
			hana::transform(subtypes, type_transform);

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = sizeof...(T);


		/// \brief hana::map from hana::type to bool
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


		/// \brief If there is only one type than the type, otherwise
		///        a std::variant of all types
		using value_type = std::conditional_t<
			type_count == 1,
			typename decltype(+types[hana::int_c< 0 >])::type,
			typename decltype(
				hana::unpack(types, hana::template_< std::variant >))::type
		>;


		/// \brief Constructor
		template < typename MakeData >
		constexpr output(MakeData const& m)
			: enabled_map_(hana::unpack(hana::transform(subtypes,
				[&](auto subtype){
					return hana::make_pair(type_transform(subtype),
						m.data.maker.enable(m.accessory, subtype));
				}), hana::make_map)) {}

		/// \brief Outputs are not copyable
		output(output const&) = delete;

		/// \brief Outputs are not movable
		output(output&&) = delete;



		/// \brief Add given data with the current id to \ref data_
		template < typename V >
		void put(V&& value){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in put< V > is not an output type"
			);

			if(!enabled_map_[hana::type_c< V >]){
				using namespace std::literals::string_literals;
				throw std::logic_error(io_tools::make_string(
					"output '", detail::to_std_string_view(name),
					"' put disabled type [", type_name< V >(), "]"
				));
			}

			data_.at(current_id()).emplace_back(static_cast< V&& >(value));
		}

		/// \brief true if any type is enabled
		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(enabled_map_));
		}

		/// \brief true if type is enabled
		template < typename U >
		constexpr bool
		is_enabled(hana::basic_type< U > const& type)const noexcept{
			auto const is_type_valid = hana::contains(enabled_map_, type);
			static_assert(is_type_valid, "type in not an input type");
			return enabled_map_[type];
		}

		/// \brief true if subtype is enabled
		template < typename U >
		constexpr bool
		is_subtype_enabled(hana::basic_type< U > const& type)const noexcept{
			return is_enabled(type_transform(type));
		}


		/// \brief Returns the output name
		virtual std::string_view get_name()const noexcept override{
			return detail::to_std_string_view(name);
		}


	protected:
		/// \brief Get a map from runtime types to bool
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
		/// \brief Get vector of references to all data with id
		virtual std::vector< reference_carrier >
		get_references(std::size_t id)const override{
			std::vector< reference_carrier > result;
			result.reserve(data_.size());

			for(auto const& data: data_.at(id)){
				if constexpr(type_count == 1){
					result.emplace_back(
						type_index::type_id< decltype(data) >(),
						reinterpret_cast< any_type const& >(data));
				}else{
					result.emplace_back(
						std::visit([](auto const& data){
							return type_index::type_id< decltype(data) >();
						}, data),
						std::visit([](auto const& data)->any_type const&{
							return reinterpret_cast< any_type const& >(data);
						}, data));
				}
			}

			return result;
		}

		/// \brief Get vector of values with all data with id
		///
		/// The data is moved into the vector!
		virtual std::vector< value_carrier >
		get_values(std::size_t id) override{
			std::vector< value_carrier > result;
			result.reserve(data_.size());

			for(auto& data: data_.at(id)){
				if constexpr(type_count == 1){
					result.emplace_back(
						type_index::type_id< decltype(data) >(),
						reinterpret_cast< any_type&& >(data));
				}else{
					result.emplace_back(
						std::visit([](auto&& data){
							return type_index::type_id< decltype(data) >();
						}, std::move(data)),
						std::visit([](auto&& data)->any_type&&{
							return reinterpret_cast< any_type&& >(data);
						}, std::move(data)));
				}
			}

			return result;
		}

		/// \brief Remove all data until the given id
		virtual void prepare()noexcept override{
			data_.try_emplace(data_.end(), current_id());
		}

		/// \brief Remove all data until the given id
		virtual void cleanup(std::size_t id)noexcept override{
			data_.erase(id);
		}


		/// \brief hana::map from type to bool, bool is true if type is enabled
		enabled_map_type enabled_map_;


		/// \brief Map from exec id to data vector
		///
		/// Since emplace and erase of map don't affect existing elements and
		/// these operations can't be called while get_references and
		/// transfer_values are running on the same exec id, all 4 operations
		/// are naturally thread safe.
		std::map< std::size_t, std::vector< value_type > > data_;
	};


}


#endif
