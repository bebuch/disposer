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

#include "config_fn.hpp"
#include "input_base.hpp"
#include "input_name.hpp"
#include "output_base.hpp"
#include "iop_list.hpp"
#include "merge.hpp"

#include <io_tools/make_string.hpp>

#include <variant>
#include <optional>
#include <unordered_map>


namespace disposer{


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

	template <
		typename InputType,
		typename ConnectionVerifyFn,
		typename TypeVerifyFn >
	struct input_maker;

	/// \brief Class input_key access key
	struct input_maker_key{
	private:
		/// \brief Constructor
		constexpr input_maker_key()noexcept = default;

		template <
			typename InputType,
			typename ConnectionVerifyFn,
			typename TypeVerifyFn >
		friend struct input_maker;
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
		constexpr input(
			output_base* output,
			bool last_use,
			std::optional< output_info > const& info
		)noexcept
			: input_base(output, last_use)
			, enabled_map_(
				hana::make_map(hana::make_pair(
					type_transform(hana::type_c< T >),
					[](std::optional< output_info > const& info, auto type){
						if(!info) return false;
						return info->is_enabled(type_transform(type));
					}(info, hana::type_c< T >)
				) ...)
			) {}


		/// \brief Get all data until the current id without transferring
		///        ownership
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

		/// \brief Get all data until the current id with transferring
		///        ownership
		std::multimap< std::size_t, values_type > get_values(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::multimap< std::size_t, values_type > result;
			if(last_use()){
				output_ptr()->transfer_values(input_key(), current_id(),
					[&result](std::vector< value_carrier >&& list){
						for(auto& carrier: list){
							result.emplace_hint(
								result.end(),
								carrier.id,
								val_convert_rt(
									carrier.type, std::move(carrier.data))
							);
						}
					});
			}else{
				for(auto& carrier: output_ptr()->get_references(
					input_key(), current_id()
				)){
					if constexpr(type_count == 1){
						result.emplace_hint(
							result.end(),
							carrier.id,
							ref_convert_rt(carrier.type, carrier.data).get()
						);
					}else{
						result.emplace_hint(
							result.end(),
							carrier.id,
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


	/// \brief Provids types for constructing an input
	template <
		typename InputType,
		typename ConnectionVerifyFn,
		typename TypeVerifyFn >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;

		/// \brief Input name as compile time string
		using name_type = typename InputType::name_type;

		/// \brief Name as hana::string
		static constexpr auto name = name_type::value;

		/// \brief Type of a disposer::input
		using type = InputType;

		/// \brief Function which verifies the connection with an output
		connection_verify_fn< ConnectionVerifyFn > connection_verify;

		/// \brief Function which verifies the active types
		type_verify_fn< TypeVerifyFn > type_verify;


		/// \brief Create an input object
		template < typename IOP_List >
		constexpr auto operator()(
			IOP_List const& iop_list,
			output_base* output,
			bool last_use,
			std::optional< output_info > const& info
		)const{
			if(info){
				constexpr auto key = input_maker_key();
				hana::unpack(type::types, [&info, key](auto ... type){
					info->verify_nothing_enabled_except(key, type ...);
				});
			}

			auto input = type(output, last_use, info);
			connection_verify(iop_list, static_cast< bool >(info));

			if(info){
				hana::for_each(type::types,
					[this, &iop_list, &info](auto const& type){
						type_verify(iop_list, type, *info);
					});
			}

			return input;
		}
	};


	/// \brief Helper function for \ref input_name::operator()
	template <
		typename Name,
		typename Types,
		typename TypeTransformFn,
		typename ConnectionVerifyFn,
		typename TypeVerifyFn >
	constexpr auto create_input_maker(
		Name const&,
		Types const&,
		type_transform_fn< TypeTransformFn >&&,
		connection_verify_fn< ConnectionVerifyFn >&& connection_verify,
		type_verify_fn< TypeVerifyFn >&& type_verify
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name, TypeTransformFn >, typelist);

		constexpr auto type_input =
			hana::unpack(unpack_types, hana::template_< input >);

		return input_maker< typename decltype(type_input)::type,
			ConnectionVerifyFn, TypeVerifyFn >{
				std::move(connection_verify),
				std::move(type_verify)
			};
	}


	template < char ... C >
	template <
		typename Types,
		typename Arg2,
		typename Arg3,
		typename Arg4 >
	constexpr auto input_name< C ... >::operator()(
		Types const& types,
		Arg2&& arg2,
		Arg3&& arg3,
		Arg4&& arg4
	)const{
		constexpr auto valid_argument = [](auto const& arg){
				return hana::is_a< type_transform_fn_tag >(arg)
					|| hana::is_a< connection_verify_fn_tag >(arg)
					|| hana::is_a< type_verify_fn_tag >(arg)
					|| hana::is_a< no_argument_tag >(arg);
			};

		auto const arg2_valid = valid_argument(arg2);
		static_assert(arg2_valid, "argument 2 is invalid");
		auto const arg3_valid = valid_argument(arg3);
		static_assert(arg3_valid, "argument 3 is invalid");
		auto const arg4_valid = valid_argument(arg4);
		static_assert(arg4_valid, "argument 4 is invalid");

		auto args = hana::make_tuple(
			static_cast< Arg2&& >(arg2),
			static_cast< Arg3&& >(arg3),
			static_cast< Arg4&& >(arg4)
		);

		auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(tt, "more than one type_transform_fn");
		auto cv = hana::count_if(args, hana::is_a< connection_verify_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(cv, "more than one connection_verify_fn");
		auto tv = hana::count_if(args, hana::is_a< type_verify_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(tv, "more than one type_verify_fn");

		return create_input_maker(
			(*this),
			types,
			get_or_default(std::move(args),
				hana::is_a< type_transform_fn_tag >,
				type_transform_fn< no_transform >{}),
			get_or_default(std::move(args),
				hana::is_a< connection_verify_fn_tag >,
				connection_verify_fn< connection_verify_always >{}),
			get_or_default(std::move(args),
				hana::is_a< type_verify_fn_tag >,
				type_verify_fn< type_verify_always >{})
		);
	}


	template < typename LocationFn, typename Makers >
	auto invalid_inputs(
		LocationFn const& location,
		Makers const& makers,
		input_list const& inputs
	){
		auto input_names = hana::transform(
			hana::filter(makers, hana::is_a< input_maker_tag >),
			[](auto const& input_maker){
				return input_maker.name;
			});

		std::set< std::string > input_name_list;
		std::transform(inputs.begin(), inputs.end(),
			std::inserter(input_name_list, input_name_list.end()),
			[](auto const& pair){ return pair.first; });
		hana::for_each(input_names,
			[&input_name_list](auto const& name){
				input_name_list.erase(name.c_str());
			});

		for(auto const& in: input_name_list){
			logsys::log([&location, &in](logsys::stdlogb& os){
				os << location << "input("
					<< in << ") doesn't exist (ERROR)";
			});
		}
		return input_name_list;
	}


}


#endif
