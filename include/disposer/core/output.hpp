//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output__hpp_INCLUDED_
#define _disposer__core__output__hpp_INCLUDED_

#include "output_base.hpp"
#include "output_name.hpp"
#include "config_fn.hpp"

#include "../tool/type_index.hpp"
#include "../tool/to_std_string_view.hpp"


namespace disposer{


	/// \brief Hana Tag for output
	struct output_tag{};

	/// \brief The actual output type
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


		/// \brief Outputs are not copyable
		output(output const&) = delete;

		/// \brief Outputs are not movable
		output(output&&) = delete;


		/// \brief Constructor
		template < typename MakeData >
		constexpr output(MakeData const& m)
			: enabled_map_(hana::unpack(hana::transform(subtypes,
				[&](auto subtype){
					return hana::make_pair(type_transform(subtype),
						m.data.maker.enable(m.accessory, subtype));
				}), hana::make_map)) {}


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
		/// \brief hana::map from type to bool, bool is true if type is enabled
		enabled_map_type enabled_map_;
	};


}


#endif
