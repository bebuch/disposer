//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__iop_accessory__hpp_INCLUDED_
#define _disposer__iop_accessory__hpp_INCLUDED_

#include "add_log.hpp"
#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"
#include "output_info.hpp"
#include "merge.hpp"

#include <iostream>


namespace disposer{


	/// \brief Hana Tag for input_maker
	struct input_maker_tag{};

	/// \brief Hana Tag for output_maker
	struct output_maker_tag{};

	/// \brief Hana Tag for parameter_maker
	struct parameter_maker_tag{};


	/// \brief Hana Tag for input
	struct input_tag{};

	/// \brief Hana Tag for output
	struct output_tag{};

	/// \brief Hana Tag for parameter
	struct parameter_tag{};


	/// \brief Log Implementation for \ref iop_ref
	struct iop_log{
		std::string_view location;
		std::string_view maker_type_name;
		std::string_view maker_name;

		void operator()(logsys::stdlogb& os)const{
			os << location << " " << maker_type_name
				<< "(" << maker_name << ") ";
		}
	};


	/// \brief Converts a IOP hana::tuple into a hana::map
	constexpr auto as_iop_map = [](auto&& xs){
		return hana::to_map(hana::transform(
			static_cast< decltype(xs)&& >(xs),
			[](auto&& x){
				return hana::make_pair(x.name, static_cast< decltype(x)&& >(x));
			}));
	};


	using namespace std::literals::string_view_literals;

	template < typename InputMaker >
	struct input_make_data{
		static constexpr auto log_name = "input"sv;

		constexpr input_make_data(
			InputMaker const& maker,
			std::optional< output_info > const& info
		)noexcept
			: maker(maker)
			, info(info){}

		constexpr input_make_data(input_make_data&& other)noexcept
			: maker(other.maker)
			, info(std::move(other.info)) {}

		InputMaker const& maker;
		std::optional< output_info > info;
	};

	template < typename Maker >
	struct output_make_data{
		static constexpr auto log_name = "output"sv;

		constexpr output_make_data(Maker const& maker)noexcept
			: maker(maker) {}

		constexpr output_make_data(output_make_data&& other)noexcept
			: maker(other.maker) {}

		Maker const& maker;
	};

	template < typename Maker, typename ValueMap >
	struct parameter_make_data{
		static constexpr auto log_name = "parameter"sv;

		constexpr parameter_make_data(
			Maker const& maker,
			ValueMap const& value_map
		)noexcept
			: maker(maker)
			, value_map(value_map) {}

		constexpr parameter_make_data(parameter_make_data&& other)noexcept
			: maker(other.maker)
			, value_map(other.value_map) {}

		Maker const& maker;
		ValueMap const value_map;
	};


	template < typename Maker >
	auto make_parameter_value_map(
		std::string_view location,
		Maker const& maker,
		parameter_list const& params
	){
		auto const name = to_std_string(maker.name);
		auto const iter = params.find(name);
		auto const found = iter != params.end();

		bool all_specialized = true;

		auto get_value =
			[&location, &all_specialized, &maker, found, name, iter](auto type)
			-> std::optional< std::string_view >
		{
			if(!found) return {};

			auto const specialization = iter->second
				.specialized_values.find(
					to_std_string(maker.to_text[type]));
			auto const end =
				iter->second.specialized_values.end();
			if(specialization == end){
				all_specialized = false;
				if(!iter->second.generic_value){
					throw std::logic_error(
						std::string(location) + "parameter("
						+ name + ") has neither a "
						"generic value but a specialization "
						"for type '" + specialization->first
						+ "'"
					);
				}else{
					return {*iter->second.generic_value};
				}
			}else{
				return {specialization->second};
			}
		};

		auto result = hana::to_map(hana::transform(
			maker.types,
			[&get_value](auto type){
				return hana::make_pair(type, get_value(type));
			}));

		if(found && all_specialized && iter->second.generic_value){
			logsys::log([&location, name](logsys::stdlogb& os){
				os << location << "parameter("
					<< name << ") has specialized values for "
					"all its types, the also given generic "
					"value will never be used (WARNING)";
			});
		}

		return result;
	}


	template < typename Maker, typename MakeData >
	auto iop_make_data(
		Maker const& maker,
		MakeData const& data,
		std::string_view location
	){
		auto is_input = hana::is_a< input_maker_tag >(maker);
		auto is_output = hana::is_a< output_maker_tag >(maker);
		auto is_parameter = hana::is_a< parameter_maker_tag >(maker);

		// silance GCC ...
		(void)is_input; (void)is_output; (void)is_parameter; (void)location;

		if constexpr(is_input){
			return disposer::input_make_data(maker,
				make_output_info(data.inputs, to_std_string(maker.name)));
		}else if constexpr(is_output){
			return disposer::output_make_data(maker);
		}else if constexpr(is_parameter){
			return disposer::parameter_make_data(maker,
				make_parameter_value_map(location, maker, data.parameters));
		}else{
			static_assert(false_c< decltype(maker) >,
				"maker is not an iop (this is a bug in disposer!)");
		}
	}

	template < typename IOP_RefList, std::size_t I >
	class iops_accessory: public add_log< iops_accessory< IOP_RefList, I > >{
	public:
		constexpr iops_accessory(
			IOP_RefList const& iop_list,
			iop_log&& log_fn
		)noexcept
			: iop_list_(iop_list)
			, log_fn_(std::move(log_fn)) {}

		/// \brief Get reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename IOP >
		auto& operator()(IOP const& iop)noexcept{
			return get(iop);
		}

		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename IOP >
		auto const& operator()(IOP const& iop)const noexcept{
			return get(iop);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			log_fn_(os);
		}

	private:
		template < typename IOP >
		auto& get(IOP const& iop)const noexcept{
			using iop_t = std::remove_reference_t< IOP >;
			static_assert(
				hana::is_a< input_name_tag, iop_t > ||
				hana::is_a< output_name_tag, iop_t > ||
				hana::is_a< parameter_name_tag, iop_t >,
				"parameter is not an input_name, output_name or "
				"parameter_name");

			using iop_tag = typename iop_t::hana_tag;

			auto iop_ref = hana::find_if(iop_list_, [&iop](auto ref){
				using tag = typename decltype(ref)::type::name_type::hana_tag;
				return hana::type_c< iop_tag > == hana::type_c< tag >
					&& ref.get().name == iop.value;
			});

			auto is_iop_valid = iop_ref != hana::nothing;
			static_assert(is_iop_valid,
				"requested iop doesn't exist (yet)");

			return iop_ref->get();
		}


		IOP_RefList iop_list_;

		/// \brief Reference to an iop_log object
		iop_log log_fn_;
	};

	template < typename IOP_RefList, typename MakeData, std::size_t I >
	struct iops_make_data{
		constexpr iops_make_data(
			MakeData&& make_data,
			std::string_view location,
			IOP_RefList const& iop_list,
			hana::size_t< I >
		)noexcept
			: data(static_cast< MakeData&& >(make_data))
			, accessory(iop_list, iop_log{
				location, make_data.log_name,
				to_std_string_view(make_data.maker.name)}) {}

		MakeData data;
		iops_accessory< IOP_RefList, I > accessory;
	};


}


#endif
