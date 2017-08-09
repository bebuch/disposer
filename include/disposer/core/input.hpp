//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input__hpp_INCLUDED_
#define _disposer__core__input__hpp_INCLUDED_

#include "input_base.hpp"
#include "output_base.hpp"
#include "config_fn.hpp"
#include "input_name.hpp"

#include <variant>
#include <optional>
#include <unordered_map>
#include <string_view>


namespace disposer{


	/// \brief Hana Tag for input
	struct input_tag{};

	template < typename Name, typename TypeTransformFn, typename ... T >
	class input;

	/// \brief Class input_key access key
	struct input_key{
	private:
		/// \brief Constructor
		constexpr input_key()noexcept = default;

		template < typename Name, typename TypeTransformFn, typename ... T >
		friend class input;
	};

	/// \brief The actual input type
	template < typename Name, typename TypeTransformFn, typename ... T >
	class input: public input_base{
	public:
		static_assert(hana::is_a< input_name_tag, Name >);


		/// \brief Hana tag to identify inputs
		using hana_tag = input_tag;


		/// \brief Compile time name of the input
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		/// \brief Meta function to transfrom subtypes to the actual types
		static constexpr auto type_transform =
			type_transform_fn< TypeTransformFn >{};

		/// \brief Subtypes (before type_transform) as hana::tuple
		static constexpr auto subtypes = hana::tuple_t< T ... >;

		/// \brief Types (after type_transform) as hana::tuple
		static constexpr auto types = hana::transform(subtypes, type_transform);

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = sizeof...(T);


		/// \brief hana::map from hana::type to bool
		using enabled_map_type = decltype(hana::make_map(
			hana::make_pair(type_transform(hana::type_c< T >), false) ...));


		static_assert(hana::length(subtypes) ==
			hana::length(hana::to_set(subtypes)),
			"disposer::input needs all subtypes T to be distinct");

		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::input needs all types T to be distinct");

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


		/// \brief Constructor
		template < typename MakeData >
		input(MakeData const& m)
			: input_base((
					// precondition call
					verify_maker_data(m.data.maker, m.accessory, m.data.info)
				, m.data.info ? m.data.info->output() : nullptr))
			, enabled_map_(
				hana::make_map(hana::make_pair(
					type_transform(hana::type_c< T >),
					[](std::optional< output_info > const& info, auto type){
						if(!info) return false;
						return info->is_enabled(type_transform(type));
					}(m.data.info, hana::type_c< T >)
				) ...)
			) {}


		/// \brief true if any type is enabled
		bool is_enabled()const noexcept{
			return hana::any(hana::values(enabled_map_));
		}

		/// \brief true if type is enabled
		template < typename U >
		bool is_enabled(hana::basic_type< U > const& type)const noexcept{
			auto const is_type_valid = hana::contains(enabled_map_, type);
			static_assert(is_type_valid, "type in not an input type");
			return enabled_map_[type];
		}

		/// \brief true if subtype is enabled
		template < typename U >
		bool is_subtype_enabled(
			hana::basic_type< U > const& type
		)const noexcept{
			return is_enabled(type_transform(type));
		}


	private:
		/// \brief Checks to make before object initialization
		template < typename InputMaker, typename Accessory >
		static void verify_maker_data(
			InputMaker const& maker,
			Accessory const& accessory,
			std::optional< output_info > const& info
		){
			if(info){
				constexpr auto key = input_key();
				hana::unpack(types, [&info, key](auto ... type){
					info->verify_nothing_enabled_except(key, type ...);
				});
			}

			maker.verify_connection(accessory, static_cast< bool >(info));

			if(info){
				hana::for_each(types,
					[&maker, &accessory, &info](auto const& type){
						maker.verify_type(accessory, type, *info);
					});
			}
		}


		/// \brief hana::map from type to bool, bool is true if type is enabled
		enabled_map_type enabled_map_;
	};


}


#endif
