//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__output_exec__hpp_INCLUDED_
#define _disposer__core__output_exec__hpp_INCLUDED_

#include "output_exec_base.hpp"
#include "output_name.hpp"
#include "config_fn.hpp"

#include "../tool/type_index.hpp"
#include "../tool/to_std_string_view.hpp"

#include <io_tools/make_string.hpp>

#include <functional>
#include <variant>


namespace disposer{


	/// \brief Hana Tag for output_exec
	struct output_exec_tag{};

	/// \brief The actual output_exec type
	template < typename Name, typename TypeTransformFn, typename ... T >
	class output_exec: public output_exec_base{
	public:
		/// \brief Hana tag to identify output_execs
		using hana_tag = output_exec_tag;


		/// \brief Corresponding output type
		using output_type = output< Name, TypeTransformFn, T ... >;


		/// \brief Compile time name of the output
		using name_type = typename output_type::name_type;

		/// \brief Name as hana::string
		static constexpr auto name = output_type::name;


		/// \brief Meta function to transfrom subtypes to the actual types
		static constexpr auto type_transform = output_type::type_transform;

		/// \brief Subtypes (before type_transform) as hana::tuple
		static constexpr auto subtypes = output_type::subtypes;

		/// \brief Types (after type_transform) as hana::tuple
		static constexpr auto types = output_type::types;

		/// \brief Count of parameter types
		static constexpr std::size_t type_count = output_type::type_count;


		/// \brief Constructor
		output_exec(std::size_t use_count)noexcept
			: output_exec_base(use_count) {}



		/// \brief If there is only one type than the type, otherwise
		///        a std::variant of all types
		using value_type = std::conditional_t<
			type_count == 1,
			typename decltype(+types[hana::int_c< 0 >])::type,
			typename decltype(
				hana::unpack(types, hana::template_< std::variant >))::type
		>;


		/// \brief Add given data to \ref data_
		template < typename V >
		void put(V&& value){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in put< V > is not an output_exec type"
			);

			if(!enabled_map_[hana::type_c< V >]){
				using namespace std::literals::string_literals;
				throw std::logic_error(io_tools::make_string(
					"output_exec '", detail::to_std_string_view(name),
					"' put disabled type [", type_name< V >(), "]"
				));
			}

			data_.emplace_back(static_cast< V&& >(value));
		}


		/// \brief Returns the output name
		virtual std::string_view get_name()noexcept const override{
			return detail::to_std_string_view(name);
		}


	private:
		/// \brief Get vector of references to all data
		virtual std::vector< reference_carrier > get_references()const override{
			std::vector< reference_carrier > result;
			result.reserve(data_.size());

			for(auto const& data: data_){
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

		/// \brief Get vector of values with all data
		///
		/// The data is moved into the vector!
		virtual std::vector< value_carrier > get_values()override{
			std::vector< value_carrier > result;
			result.reserve(data_.size());

			for(auto& data: data_){
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

		/// \brief Remove all data
		virtual void cleanup()noexcept override{
			data_.clear();
		}


		/// \brief Putted data of the output
		std::vector< value_type > data_;
	};


}


#endif
