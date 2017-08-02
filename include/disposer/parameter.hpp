//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__parameter__hpp_INCLUDED_
#define _disposer__parameter__hpp_INCLUDED_

#include "config_fn.hpp"

#include "tool/type_index.hpp"
#include "tool/to_std_string.hpp"
#include "tool/to_std_string_view.hpp"

#include <io_tools/make_string.hpp>

#include <optional>


namespace disposer{


	/// \brief Hana Tag for \ref parameter_name
	struct parameter_name_tag{};

	/// \brief Hana Tag for parameter_maker
	struct parameter_maker_tag{};

	/// \brief Hana Tag for parameter
	struct parameter_tag{};

	template < typename Name, typename TypeTransformFn, typename ... T >
	class parameter{
	public:
		static_assert(hana::is_a< parameter_name_tag, Name >);


		/// \brief Hana tag to identify parameters
		using hana_tag = parameter_tag;


		/// \brief Compile time name of the parameter
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


		static_assert(hana::length(subtypes) ==
			hana::length(hana::to_set(subtypes)),
			"disposer::parameter needs all subtypes T to be distinct");

		static_assert(hana::length(types) == hana::length(hana::to_set(types)),
			"disposer::parameter needs all types T to be distinct");

		static_assert(type_count != 0,
			"disposer::parameter needs at least on type");

		static_assert(!hana::any_of(subtypes, hana::traits::is_const),
			"disposer::parameter subtypes must not be const");

		static_assert(!hana::any_of(subtypes, hana::traits::is_reference),
			"disposer::parameter subtypes must not be references");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::parameter types must not be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::parameter types must not be references");


		using type_value_map_t =
			decltype(hana::make_map(hana::make_pair(
				type_transform(hana::type_c< T >),
				std::declval< std::optional<
					typename decltype(type_transform(hana::type_c< T >))::type
				> >()) ... ));


		template <
			typename Maker,
			typename Accessory,
			typename Type >
		static std::optional< Type > make_value(
			Maker const& maker,
			Accessory const& accessory,
			std::optional< std::string_view > value,
			std::string const& name,
			hana::basic_type< Type > type
		){
			if(!maker.enable(accessory, type,
				detail::to_std_string_view(maker.to_text[type]))) return {};
			if(value) return maker.parser(accessory, *value, type);
			auto const have_default_fn =
				!maker.default_value_generator.is_void_r(accessory, type);
			if constexpr(have_default_fn){
				auto value = maker.default_value_generator(accessory, type);
				if(value) return *value;
			}

			std::ostringstream os;
			os << "parameter(" << name << ") of type [" << type_name< Type >()
				<< "] is required";
			throw std::logic_error(os.str());
		}


		/// \brief Constructor
		template < typename MakeData >
		parameter(MakeData const& m):
			type_value_map_(hana::unpack(hana::transform(types, [&](auto type){
					auto value = make_value(
						m.data.maker, m.accessory, m.data.value_map[type],
						detail::to_std_string(m.data.maker.name), type);
					if(value){
						m.data.maker.verify_value(m.accessory, *value);
					}
					return hana::make_pair(type, std::move(value));
				}), hana::make_map)) {}

		/// \brief Parameters are not copyable
		parameter(parameter const&) = delete;

		/// \brief Parameters are not movable
		parameter(parameter&&) = delete;


		/// \brief true if any type is enabled, otherwise false
		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(type_value_map_));
		}

		/// \brief true if type is enabled, otherwise false
		template < typename Type >
		bool is_enabled(Type const& type)const{
			auto const is_type_valid = hana::contains(type_value_map_, type);
			static_assert(is_type_valid, "type in not an input type");
			return static_cast< bool >(type_value_map_[type]);
		}


		/// \brief Access the value if parameter has only one type
		decltype(auto) get()const{
			static_assert(type_count == 1,
				"you must call with a type: get(hana::type_c< Type >)");
			return get(types[hana::int_c< 0 >]);
		}

		/// \brief Access parameter of given type
		template < typename Type >
		decltype(auto) get(Type const& type)const{
			if(!is_enabled(type)){
				throw std::logic_error(io_tools::make_string(
					"accessed parameter '", detail::to_std_string_view(name),
					"' with disabled type [",
					type_name< typename Type::type >(), "]"
				));
			}
			return *type_value_map_[type];
		}


	private:
		/// \brief Map of parameter types to values
		type_value_map_t type_value_map_;
	};


	struct default_value_type_impl{
		template < typename ... T >
		constexpr auto operator()(T ... v)const noexcept{
			return hana::type_c< std::optional< decltype(hana::make_map(
				hana::make_pair(v, std::declval< typename T::type >()) ...
			)) > >;
		}
	};

	template < typename Types >
	using default_value_type = typename decltype(
		hana::unpack(Types{}, default_value_type_impl{}))::type;


}


#endif
