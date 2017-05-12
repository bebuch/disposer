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
#include "input_name.hpp"
#include "output_base.hpp"
#include "iop_list.hpp"

#include <io_tools/make_string.hpp>

#include <variant>
#include <optional>
#include <unordered_map>


namespace disposer{


	template < typename Name, typename TypesMetaFn, typename ... T >
	class input;

	/// \brief Class input_key access key
	struct input_key{
	private:
		/// \brief Constructor
		constexpr input_key()noexcept = default;

		template < typename Name, typename TypesMetaFn, typename ... T >
		friend class input;
	};


	template < typename Name, typename TypesMetaFn, typename ... T >
	class input: public input_base{
	public:
		static_assert(hana::is_a< input_name_tag, Name >);
		static_assert(hana::Metafunction< TypesMetaFn >::value);


		using hana_tag = input_tag;


		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;


		static constexpr auto subtypes = hana::tuple_t< T ... >;

		static constexpr auto types =
			hana::transform(subtypes, TypesMetaFn{});

		static constexpr std::size_t type_count = sizeof...(T);


		using enabled_map_type = decltype(hana::make_map(
			hana::make_pair(hana::type_c< T >, false) ...));


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


		using values_type = std::conditional_t<
			type_count == 1,
			typename decltype(+types[hana::int_c< 0 >])::type,
			decltype(hana::unpack(types, hana::template_< std::variant >))
		>;

		using references_type = std::conditional_t<
			type_count == 1,
			std::reference_wrapper<
				typename decltype(+types[hana::int_c< 0 >])::type >,
			decltype(hana::unpack(
				hana::transform(
					hana::transform(types, hana::traits::add_const),
					hana::template_< std::reference_wrapper >),
				hana::template_< std::variant >))
		>;


		constexpr input(output_base* output, bool last_use)noexcept:
			input_base(Name::value.c_str(), output), last_use_(last_use) {}


		std::multimap< std::size_t, references_type > get_references(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::multimap< std::size_t, references_type > result;
			for(auto& carrier: output_ptr()->get_references(
				input_key(), current_id()
			)){
				result.emplace_hint(
					result.end(),
					carrier.id,
					ref_convert_rt(carrier.type, carrier.data)
				);
			}
			return result;
		}

		std::multimap< std::size_t, values_type > get_values(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::multimap< std::size_t, values_type > result;
			if(last_use_){
				output_ptr()->transfer_values(input_key(), current_id(),
					[&result](std::vector< value_carrier >&& list){
						for(auto& carrier: list){
							result.emplace_hint(
								result.end(),
								carrier.id,
								val_convert_rt(carrier.type, carrier.data)
							);
						}
					});
			}else{
				for(auto& carrier: output_ptr()->get_references(
					input_key(), current_id()
				)){
					result.emplace_hint(
						result.end(),
						carrier.id,
						ref_convert_rt(carrier.type, carrier.data).get()
					);
				}
			}
			return result;
		}


		constexpr bool is_enabled()const noexcept{
			return hana::any(hana::values(enabled_map_));
		}

		template < typename U >
		constexpr bool
		is_enabled(hana::basic_type< U > const& type)const noexcept{
			return enabled_map_[type];
		}


	private:
		using ref_convert_fn = references_type(*)(any_type const& data);
		using val_convert_fn = values_type(*)(any_type&& data);


		static std::unordered_map< type_index, ref_convert_fn > const ref_map_;
		static std::unordered_map< type_index, val_convert_fn > const val_map_;


		template < typename U >
		static references_type ref_convert(any_type const& data)noexcept{
			return std::cref(reinterpret_cast< U const& >(data));
		}

		template < typename U >
		static values_type val_convert(any_type&& data){
			return reinterpret_cast< U&& >(data);
		}

		static references_type ref_convert_rt(
			type_index const& type,
			any_type const& data
		)noexcept{
			auto iter = ref_map_.find(type);
			assert(iter != ref_map_.end());
			return (iter->second)(data);
		}

		static values_type val_convert_rt(
			type_index const& type,
			any_type&& data
		){
			auto iter = val_map_.find(type);
			assert(iter != val_map_.end());
			return (iter->second)(data);

		}


		virtual std::vector< type_index > type_list()const override{
			std::vector< type_index > result;
			result.reserve(type_count);
			hana::for_each(enabled_map_, [&result](auto const& pair){
				using type = typename decltype(+hana::first(pair))::type;
				result.push_back(type_index::type_id_with_cvr< type >());
			});
			return result;
		}

		virtual bool enable_types(
			std::vector< type_index > const& types
		)noexcept override{
			for(auto const& type: types){
				auto iter = rt_enabled_map_.find(type);

				if(iter == rt_enabled_map_.end()) return false;

				iter->second.get() = true;
			}

			return true;
		}


		enabled_map_type enabled_map_;

		std::unordered_map< type_index, std::reference_wrapper< bool > > const
			rt_enabled_map_ = { {
				type_index::type_id_with_cvr< T >(),
				enabled_map_[hana::type_c< T >]
			} ... };


		bool const last_use_;
	};


	template < typename Name, typename TypesMetaFn, typename ... T >
	std::unordered_map< type_index,
		typename input< Name, TypesMetaFn, T ... >::ref_convert_fn > const
		input< Name, TypesMetaFn, T ... >::ref_map_ = {
			{
				type_index::type_id_with_cvr< T >(),
				&input< Name, TypesMetaFn, T ... >::ref_convert< T >
			} ...
		};

	template < typename Name, typename TypesMetaFn, typename ... T >
	std::unordered_map< type_index,
		typename input< Name, TypesMetaFn, T ... >::val_convert_fn > const
		input< Name, TypesMetaFn, T ... >::val_map_ = {
			{
				type_index::type_id_with_cvr< T >(),
				&input< Name, TypesMetaFn, T ... >::val_convert< T >
			} ...
		};


	/// \brief Provid types for constructing an input
	template <
		typename Name,
		typename InputType,
		typename VerifyConnectFn,
		typename VerifyTypesFn >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;

		/// \brief Input name as compile time string
		using name_type = Name;

		/// \brief Name as hana::string
		static constexpr auto name = Name::value;

		/// \brief Type of a disposer::input
		using type = InputType;

		/// \brief Function which verifies the connection with an output
		VerifyConnectFn verify_connect_fn;

		/// \brief Function which verifies the active types
		VerifyTypesFn verify_type_fn;


		template < typename IOP_List >
		constexpr auto operator()(
			IOP_List const& iop_list,
			output_base* output,
			bool last_use,
			std::optional< output_info > const& info
		)const{
			auto input = type(output, last_use);
			verify_connect_fn(iop_list, static_cast< bool >(info));

			if(info){
				hana::for_each(input.types,
					[this, &iop_list, &info](auto const& type){
						verify_type_fn(iop_list, type, *info);
					});
			}

			return input;
		}
	};


	template < char ... C >
	template <
		typename Types,
		typename TypesMetaFn,
		typename VerifyConnectFn,
		typename VerifyTypesFn >
	constexpr auto input_name< C ... >::operator()(
		Types const&,
		TypesMetaFn const&,
		VerifyConnectFn&& verify_connect_fn,
		VerifyTypesFn&& verify_type_fn
	)const noexcept{
		using name_type = input_name< C ... >;
		using type_fn = std::remove_const_t< TypesMetaFn >;
		using verify_connect_fn_t =
			std::remove_reference_t< VerifyConnectFn >;
		using verify_type_fn_t =
			std::remove_reference_t< VerifyTypesFn >;

		static_assert(hana::Metafunction< TypesMetaFn >::value,
			"TypesMetaFn must model boost::hana::Metafunction");

		constexpr auto typelist = to_typelist(Types{});

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< name_type, type_fn >, typelist);

		constexpr auto type_input =
			hana::unpack(unpack_types, hana::template_< input >);

		return input_maker< name_type, typename decltype(type_input)::type,
			verify_connect_fn_t, verify_type_fn_t >{
				static_cast< VerifyConnectFn&& >(verify_connect_fn),
				static_cast< VerifyTypesFn&& >(verify_type_fn)
			};
	}

}


#endif
