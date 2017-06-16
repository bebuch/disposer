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

#include "output_base.hpp"
#include "type_index.hpp"
#include "output_name.hpp"
#include "iop_list.hpp"
#include "merge.hpp"

#include <io_tools/make_string.hpp>

#include <functional>
#include <variant>
#include <mutex>


namespace disposer{


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
		constexpr output(enabled_map_type&& enable_map)noexcept:
			enabled_map_(std::move(enable_map))
			{}

		/// \brief Outputs are default-movable
		constexpr output(output&& other):
			enabled_map_(std::move(other.enabled_map_)),
			data_(std::move(other.data_)){}


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
					"output '", to_std_string_view(name),
					"' put disabled type [",
					type_name< V >(), "]"
				));
			}

			data_.emplace(current_id(), static_cast< V&& >(value));
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
			return to_std_string_view(name);
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
		/// \brief Get references of all data until the given id
		virtual std::vector< reference_carrier >
		get_references(std::size_t id)override{
			std::lock_guard< std::mutex > lock(mutex_);

			auto from = data_.begin();
			auto const to = data_.upper_bound(id);

			std::vector< reference_carrier > result;
			result.reserve(std::distance(from, to));

			for(; from != to; ++from){
				if constexpr(type_count == 1){
					result.emplace_back(
						from->first,
						type_index::type_id< decltype(from->second) >(),
						reinterpret_cast< any_type const& >(from->second));
				}else{
					result.emplace_back(
						from->first,
						std::visit([](auto const& data){
							return type_index::type_id< decltype(data) >();
						}, from->second),
						std::visit([](auto const& data)->any_type const&{
							return reinterpret_cast< any_type const& >(data);
						}, from->second));
				}
			}

			return result;
		}

		/// \brief Call fn with a vector of all data until the given id
		///
		/// The data is moved into the vector!
		virtual void transfer_values(
			std::size_t id,
			TransferFn const& fn
		)override{
			std::lock_guard< std::mutex > lock(mutex_);

			auto from = data_.begin();
			auto const to = data_.upper_bound(id);

			std::vector< value_carrier > result;
			result.reserve(std::distance(from, to));

			for(; from != to; ++from){
				if constexpr(type_count == 1){
					result.emplace_back(
						from->first,
						type_index::type_id< decltype(from->second) >(),
						reinterpret_cast< any_type&& >(from->second));
				}else{
					result.emplace_back(
						from->first,
						std::visit([](auto&& data){
							return type_index::type_id< decltype(data) >();
						}, std::move(from->second)),
						std::visit([](auto&& data)->any_type&&{
							return reinterpret_cast< any_type&& >(data);
						}, std::move(from->second)));
				}
			}

			fn(std::move(result));
		}

		/// \brief Remove all data until the given id
		virtual void cleanup(std::size_t id)noexcept override{
			std::lock_guard< std::mutex > lock(mutex_);
			data_.erase(data_.begin(), data_.upper_bound(id));
		}


		/// \brief Protect \ref data_
		std::mutex mutex_;

		/// \brief hana::map from type to bool, bool is true if type is enabled
		enabled_map_type enabled_map_;

		/// \brief Map from id to data
		std::multimap< std::size_t, value_type > data_;
	};


	/// \brief Provid types for constructing an output
	template <
		typename OutputType,
		typename EnableFn >
	struct output_maker{
		/// \brief Tag for boost::hana
		using hana_tag = output_maker_tag;

		/// \brief Output name as compile time string
		using name_type = typename OutputType::name_type;

		/// \brief Name as hana::string
		static constexpr auto name = name_type::value;

		/// \brief Type of a disposer::output
		using type = OutputType;

		/// \brief Enable function
		enable_fn< EnableFn > enable;

		/// \brief Create an output object
		template < typename IOP_List >
		constexpr auto operator()(IOP_List const& iop_list)const{
			return type(hana::unpack(hana::transform(type::subtypes,
				[&](auto subtype){
					return hana::make_pair(type::type_transform(subtype),
						enable(iop_list, subtype));
				}), hana::make_map));
		}
	};


	/// \brief Helper function for \ref output_name::operator()
	template <
		typename Name,
		typename Types,
		typename TypeTransformFn,
		typename EnableFn >
	constexpr auto create_output_maker(
		Name const&,
		Types const&,
		type_transform_fn< TypeTransformFn >&&,
		enable_fn< EnableFn >&& enable
	){
		constexpr auto typelist = to_typelist(Types{});

		constexpr auto unpack_types =
			hana::concat(hana::tuple_t< Name, TypeTransformFn >, typelist);

		constexpr auto type_output =
			hana::unpack(unpack_types, hana::template_< output >);

		return output_maker<
			typename decltype(type_output)::type, EnableFn >{
				std::move(enable)
			};
	}


	template < char ... C >
	template <
		typename Types,
		typename Arg2,
		typename Arg3 >
	constexpr auto output_name< C ... >::operator()(
		Types const& types,
		Arg2&& arg2,
		Arg3&& arg3
	)const{
		constexpr auto valid_argument = [](auto const& arg){
				return hana::is_a< type_transform_fn_tag >(arg)
					|| hana::is_a< enable_fn_tag >(arg)
					|| hana::is_a< no_argument_tag >(arg);
			};

		auto const arg2_valid = valid_argument(arg2);
		static_assert(arg2_valid, "argument 2 is invalid");
		auto const arg3_valid = valid_argument(arg3);
		static_assert(arg3_valid, "argument 3 is invalid");

		auto args = hana::make_tuple(
			static_cast< Arg2&& >(arg2),
			static_cast< Arg3&& >(arg3)
		);

		auto tt = hana::count_if(args, hana::is_a< type_transform_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(tt, "more than one type_transform_fn");
		auto ef = hana::count_if(args, hana::is_a< enable_fn_tag >)
			<= hana::size_c< 1 >;
		static_assert(ef, "more than one enable_fn");

		return create_output_maker(
			(*this),
			types,
			get_or_default(std::move(args),
				hana::is_a< type_transform_fn_tag >,
				type_transform_fn< no_transform >{}),
			get_or_default(std::move(args),
				hana::is_a< enable_fn_tag >,
				enable_fn< enable_always >{})
		);
	}


	template < typename LocationFn, typename Makers >
	auto invalid_outputs(
		LocationFn const& location,
		Makers const& makers,
		output_list const& outputs
	){
		auto output_names = hana::transform(
			hana::filter(makers, hana::is_a< output_maker_tag >),
			[](auto const& output_maker){
				return output_maker.name;
			});

		auto output_name_list = outputs;
		hana::for_each(output_names,
			[&output_name_list](auto const& name){
				output_name_list.erase(to_std_string(name));
			});

		for(auto const& out: output_name_list){
			logsys::log([&location, &out](logsys::stdlogb& os){
				os << location << "output("
					<< out << ") doesn't exist (ERROR)";
			});
		}
		return output_name_list;
	}


}


#endif
