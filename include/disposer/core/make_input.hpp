//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__make_input__hpp_INCLUDED_
#define _disposer__core__make_input__hpp_INCLUDED_

#include "input_name.hpp"
#include "dimension_solve.hpp"
#include "output_base.hpp"

#include "../config/module_make_data.hpp"

#include "../tool/to_std_string_view.hpp"

#include <array>
#include <variant>
#include <unordered_map>


namespace disposer{


	using namespace std::literals::string_view_literals;


	/// \brief Provids types for constructing an input
	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired >
	struct input_maker{
		/// \brief Tag for boost::hana
		using hana_tag = input_maker_tag;
	};


	/// \brief Configuration class for inputs
	template < bool IsRequired >
	struct is_required: std::bool_constant< IsRequired >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto required = is_required< true >{};

	/// \brief Used as make-function argument of inputs
	constexpr auto not_required = is_required< false >{};


	/// \brief Creates a \ref input_maker object
	template <
		char ... C,
		template < typename ... > typename Template,
		std::size_t ... D,
		bool IsRequired = true >
	constexpr auto make(
		input_name< C ... > const&,
		dimension_referrer< Template, D ... > const&,
		is_required< IsRequired > = required
	){
		return input_maker<
			input_name< C ... >,
			dimension_referrer< Template, D ... >,
			IsRequired >{};
	}


	inline output_base* get_output_ptr(
		input_list const& inputs,
		std::string_view const& name
	){
		auto const iter = inputs.find(name);
		return iter != inputs.end() ? iter->second : nullptr;
	}


	template < typename Name, bool IsRequired, typename Types >
	using input_variant = typename
		decltype(input_variant_type< Name, IsRequired >(Types{}))::type;

	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired,
		typename ... Ds,
		bool ... KDs,
		typename ... Ts >
	auto deduce_dimensions_by_input(
		input_maker< Name, DimensionReferrer, IsRequired >,
		dimension_list< Ds ... >,
		output_base* const output_ptr
	){
		if constexpr(IsRequired){
			constexpr dimension_solver solver(
				dimension_list< Ds ... >{},
				DimensionReferrer{});
			return solver.solve(Name{}, output_ptr->get_type()));
		}else{
			(void)output_ptr; (void)dims; // Silance GCC
			return solved_dimensions{};
		}
	}


	template < typename PartialDeducedDimensionList >
	struct input_construction{
		template <
			typename Name,
			typename DimensionReferrer,
			bool IsRequired,
			typename ... Config,
			typename ... IOPs,
			typename StateMakerFn,
			typename ExecFn,
			std::size_t ... SDs >
		static std::unique_ptr< module_base > make(
			input_maker< Name, DimensionReferrer, IsRequired > const& maker,
			module_configure< Config ... > const& configs,
			accessory< IOPs ... >&& iops,
			module_make_data const& data,
			std::string_view location,
			state_maker_fn< StateMakerFn > const& state_maker,
			exec_fn< ExecFn > const& exec,
			solved_dimensions< SDs ... > const& solved_dims,
			output_base* const output_ptr
		){
			if constexpr(solved_dims){
				using make_fn_type = std::unique_ptr< module_base >(*)(
						input_maker< Name, DimensionReferrer, IsRequired >
							const&,
						module_configure< Config ... > const&,
						accessory< IOPs ... >&&,
						module_make_data const&,
						std::string_view,
						state_maker_fn< StateMakerFn > const&,
						exec_fn< ExecFn > const&,
						decltype(solved_dims.rest()) const&,
						output_base* const
					);

				auto const solved_d = solved_dims.dimension_number();

				constexpr auto tc = PartialDeducedDimensionList
					::dimensions[solved_d].type_count;

				constexpr auto call = hana::unpack(
					hana::range_c< std::size_c, 0, tc >,
					[](auto ... i){
						template < std::size_t I >
						using next_list_type =
							decltype(reduce_dimension_list(
								PartialDeducedDimensionList{},
								ct_index_component< solved_d.value, I >{}
							));

						return std::array< make_fn_type, sizeof...(i) >{{
								&input_construction< next_list_type
									< decltype(i)::value > >::make ...
							}};
					});

				call[solved_dims.index_number()](
						maker,
						configs,
						std::move(iops),
						data,
						location,
						state_maker,
						exec,
						solved_dims.rest(),
						output_ptr
					);
			}else{
				auto constexpr converter = DimensionReferrer::
					template convert< PartialDeducedDimensionList >;

				if(IsRequired || output_ptr != nullptr){
					auto const type = output_ptr->get_type();
					auto const active_type = converter.to_type_index(
						packed_index{dims, converter.packed});

					if(type != active_type){
						throw std::logic_error("type of input is ["
							+ active_type.pretty_name()
							+ "] but connected output is of type ["
							+ type.pretty_name() + "]");
					}
				}

				auto const make_fn = [](auto t){
						return [](output_base* output_ptr)->result_type{
							return input_construct_data< input_name< Cs ... >, typename
								decltype(t)::type, IsRequired >{output_ptr};
						};
					};

				auto const type_to_data = hana::unpack(converter.types,
					[make_fn](auto ... t){
						return std::unordered_map<
								type_index, result_type(*)(output_base*)
							>{{
								type_index::type_id< typename decltype(t)::type >(),
								make_fn(t)
							} ...};
					});

				auto const active_type = converter.to_type_index(
					packed_index{dims, converter.packed});

				input< input_name< Cs ... >, Type, IsRequired > input{output_ptr};

				return make_module(/* TODO subcall */);
			}
		}
	};

	template <
		typename Name,
		typename DimensionReferrer,
		bool IsRequired,
		typename ... Ds,
		typename ... Config,
		typename ... IOPs,
		typename StateMakerFn,
		typename ExecFn >
	std::unique_ptr< module_base > exec_make_input(
		input_maker< Name, DimensionReferrer, IsRequired > const& maker,
		dimension_list< Ds ... > const& dims,
		module_configure< Config ... > const& configs,
		accessory< IOPs ... >&& iops,
		module_make_data const& data,
		std::string_view location,
		state_maker_fn< StateMakerFn > const& state_maker,
		exec_fn< ExecFn > const& exec
	){
		auto const output_ptr = get_output_ptr(data.inputs,
			detail::to_std_string_view(Name{}));

		if constexpr(IsRequired){
			if(output_ptr == nullptr){
				throw std::logic_error("input is required but not set");
			}
		}

		return input_construction< dimension_list< Ds ... > >
			::call_construct_input(maker, configs, std::move(iops), data,
				location, state_maker, exec, deduce_dimensions_by_input(
					maker, dims, output_ptr), output_ptr);
	}


}


#endif
