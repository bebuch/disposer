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

#include <variant>
#include <optional>
#include <unordered_map>
#include <string_view>


namespace disposer{


	/// \brief Hana Tag for \ref input_name
	struct input_name_tag{};

	/// \brief Hana Tag for input_maker
	struct input_maker_tag{};

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


		/// \brief If there is only one type than the type, otherwise
		///        a std::variant of all types
		using values_type = std::conditional_t<
			type_count == 1,
			typename decltype(+types[hana::int_c< 0 >])::type,
			typename decltype(hana::unpack(
				types, hana::template_< std::variant >))::type
		>;

		/// \brief If there is only one type than a std::reference_wrapper of
		///        the type, otherwise a std::variant of
		///        std::reference_wrapper's of all types
		using references_type = std::conditional_t<
			type_count == 1,
			std::reference_wrapper<
				typename decltype(+types[hana::int_c< 0 >])::type const >,
			typename decltype(hana::unpack(
				hana::transform(
					hana::transform(types, hana::traits::add_const),
					hana::template_< std::reference_wrapper >),
				hana::template_< std::variant >))::type
		>;


		/// \brief Constructor
		template < typename MakeData >
		constexpr input(MakeData const& m)
			: input_base(
				(verify_maker_data(
						m.data.maker, m.accessory, m.data.info
					) // precondition call
					, m.data.info ? m.data.info->output() : nullptr),
				m.data.info ? m.data.info->last_use() : true)
			, enabled_map_(
				hana::make_map(hana::make_pair(
					type_transform(hana::type_c< T >),
					[](std::optional< output_info > const& info, auto type){
						if(!info) return false;
						return info->is_enabled(type_transform(type));
					}(m.data.info, hana::type_c< T >)
				) ...)
			) {}

		/// \brief Inputs are not copyable
		input(input const&) = delete;

		/// \brief Inputs are not movable
		input(input&&) = delete;


		/// \brief Get all data until the current id without transferring
		///        ownership
		std::vector< references_type > get_references(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::vector< references_type > result;
			for(auto const& carrier: output_ptr()->get_references(
				input_key(), current_id()
			)){
				result.emplace_back(
					ref_convert_rt(carrier.type, carrier.data)
				);
			}
			return result;
		}

		/// \brief Get all data until the current id with transferring
		///        ownership
		std::vector< values_type > get_values(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::vector< values_type > result;
			if(last_use()){
				for(auto& carrier: output_ptr()->get_values(
					input_key(), current_id()
				)){
					result.emplace_back(
						val_convert_rt(carrier.type, std::move(carrier.data))
					);
				};
			}else{
				for(auto const& carrier: output_ptr()->get_references(
					input_key(), current_id()
				)){
					// copy from std::reference_wrapper wrapper needs condition
					if constexpr(type_count == 1){
						result.emplace_back(
							ref_convert_rt(carrier.type, carrier.data).get()
						);
					}else{
						result.emplace_back(
							std::visit([](auto const& ref)->values_type{
									return ref.get();
								},
								ref_convert_rt(carrier.type, carrier.data)
							)
						);
					}
				}
			}
			return result;
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


	private:
		/// \brief Checks to make before object initialization
		template < typename InputMaker, typename Accessory >
		static constexpr void verify_maker_data(
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


		/// \brief Pointer to function to convert \ref any_type to
		///        \ref references_type
		using ref_convert_fn = references_type(*)(any_type const& data);

		/// \brief Pointer to function to convert \ref any_type to
		///        \ref values_type
		using val_convert_fn = values_type(*)(any_type&& data);


		/// \brief Map from runtime_type to conversion function
		///        (\ref any_type to \ref references_type)
		static std::unordered_map< type_index, ref_convert_fn > const ref_map_;

		/// \brief Map from runtime_type to conversion function
		///        (\ref any_type to \ref values_type)
		static std::unordered_map< type_index, val_convert_fn > const val_map_;


		/// \brief Conversion function from \ref any_type to
		///        \ref references_type
		template < typename U >
		static references_type ref_convert(any_type const& data)noexcept{
			return std::cref(reinterpret_cast< U const& >(data));
		}

		/// \brief Conversion function from \ref any_type to \ref values_type
		template < typename U >
		static values_type val_convert(any_type&& data){
			return reinterpret_cast< U&& >(data);
		}


		/// \brief Conversion function from runtime type to
		///        \ref references_type
		static references_type ref_convert_rt(
			type_index const& type,
			any_type const& data
		)noexcept{
			auto iter = ref_map_.find(type);
			assert(iter != ref_map_.end());
			return (iter->second)(data);
		}

		/// \brief Conversion function from runtime type to
		///        \ref values_type
		static values_type val_convert_rt(
			type_index const& type,
			any_type&& data
		){
			auto iter = val_map_.find(type);
			assert(iter != val_map_.end());
			return (iter->second)(std::move(data));
		}


		/// \brief hana::map from type to bool, bool is true if type is enabled
		enabled_map_type enabled_map_;
	};


	template < typename Name, typename TypeTransformFn, typename ... T >
	std::unordered_map< type_index,
		typename input< Name, TypeTransformFn, T ... >::ref_convert_fn > const
		input< Name, TypeTransformFn, T ... >::ref_map_ = {
			{
				type_index::type_id_with_cvr<
					typename type_transform_fn< TypeTransformFn >
						::template apply< T >::type >(),
				&input< Name, TypeTransformFn, T ... >::ref_convert<
					typename type_transform_fn< TypeTransformFn >
						::template apply< T >::type >
			} ...
		};

	template < typename Name, typename TypeTransformFn, typename ... T >
	std::unordered_map< type_index,
		typename input< Name, TypeTransformFn, T ... >::val_convert_fn > const
		input< Name, TypeTransformFn, T ... >::val_map_ = {
			{
				type_index::type_id_with_cvr<
					typename type_transform_fn< TypeTransformFn >::
						template apply< T >::type >(),
				&input< Name, TypeTransformFn, T ... >::val_convert<
					typename type_transform_fn< TypeTransformFn >::
						template apply< T >::type >
			} ...
		};


}


#endif
