//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input_exec__hpp_INCLUDED_
#define _disposer__core__input_exec__hpp_INCLUDED_

#include "input_exec_base.hpp"
#include "input.hpp"


namespace disposer{


	/// \brief Hana Tag for input_exec
	struct input_exec_tag{};

	/// \brief The actual input_exec type
	template < typename Name, typename TypeTransformFn, typename ... T >
	class input_exec: public input_exec_base{
	public:
		/// \brief Hana tag to identify input_execs
		using hana_tag = input_exec_tag;


		/// \brief Corresponding input type
		using input_type = input< Name, TypeTransformFn, T ... >;


		/// \brief Compile time name of the input
		using name_type = typename input_type::name_type;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Meta function to transfrom subtypes to the actual types
		static constexpr auto type_transform =input_type::type_transform;

		/// \brief Subtypes (before type_transform) as hana::tuple
		static constexpr auto subtypes = input_type::subtypes;

		/// \brief Types (after type_transform) as hana::tuple
		static constexpr auto types = input_type::types;

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = input_type::type_count;


		/// \brief Constructor
		input_exec(output_exec_base* output)noexcept
			: input_exec_base(output) {}


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


		/// \brief Get all data without transferring ownership
		std::vector< references_type > get_references(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::vector< references_type > result;
			for(auto const& carrier:
				output_ptr()->get_references(input_exec_key())
			){
				result.emplace_back(
					ref_convert_rt(carrier.type, carrier.data)
				);
			}
			return result;
		}

		/// \brief Get all data with transferring ownership
		std::vector< values_type > get_values(){
			if(!output_ptr()){
				throw std::logic_error("input is not linked to an output");
			}

			std::vector< values_type > result;
			TODO;
			if(is_last_use()){
				for(auto& carrier: output_ptr()->get_values(
					input_exec_key()
				)){
					result.emplace_back(
						val_convert_rt(carrier.type, std::move(carrier.data))
					);
				};
			}else{
				for(auto const& carrier: output_ptr()->get_references(
					input_exec_key()
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
	};


	template < typename Name, typename TypeTransformFn, typename ... T >
	std::unordered_map< type_index,
		typename input_exec< Name, TypeTransformFn, T ... >::ref_convert_fn > const
		input_exec< Name, TypeTransformFn, T ... >::ref_map_ = {
			{
				type_index::type_id_with_cvr<
					typename type_transform_fn< TypeTransformFn >
						::template apply< T >::type >(),
				&input_exec< Name, TypeTransformFn, T ... >::ref_convert<
					typename type_transform_fn< TypeTransformFn >
						::template apply< T >::type >
			} ...
		};

	template < typename Name, typename TypeTransformFn, typename ... T >
	std::unordered_map< type_index,
		typename input_exec< Name, TypeTransformFn, T ... >::val_convert_fn > const
		input_exec< Name, TypeTransformFn, T ... >::val_map_ = {
			{
				type_index::type_id_with_cvr<
					typename type_transform_fn< TypeTransformFn >::
						template apply< T >::type >(),
				&input_exec< Name, TypeTransformFn, T ... >::val_convert<
					typename type_transform_fn< TypeTransformFn >::
						template apply< T >::type >
			} ...
		};


}


#endif
