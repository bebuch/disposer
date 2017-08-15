//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__accessory__hpp_INCLUDED_
#define _disposer__core__accessory__hpp_INCLUDED_

#include "input_name.hpp"
#include "output_name.hpp"
#include "parameter_name.hpp"
#include "output_info.hpp"

#include "../tool/add_log.hpp"
#include "../tool/extract.hpp"
#include "../tool/false_c.hpp"
#include "../tool/to_std_string.hpp"
#include "../tool/to_std_string_view.hpp"

#include <boost/hana/map.hpp>


namespace disposer{


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

		input_make_data(
			InputMaker const& maker,
			std::optional< output_info > const& info
		)noexcept
			: maker(maker)
			, info(info){}

		InputMaker const& maker;
		std::optional< output_info > info;
	};

	template < typename Maker >
	struct output_make_data{
		static constexpr auto log_name = "output"sv;

		output_make_data(Maker const& maker, std::size_t use_count)noexcept
			: maker(maker)
			, use_count(use_count) {}

		Maker const& maker;
		std::size_t const use_count;
	};

	template < typename Maker, typename ValueMap >
	struct parameter_make_data{
		static constexpr auto log_name = "parameter"sv;

		parameter_make_data(
			Maker const& maker,
			ValueMap const& value_map
		)noexcept
			: maker(maker)
			, value_map(value_map) {}

		Maker const& maker;
		ValueMap const value_map;
	};


	auto inline get_use_count(
		output_list const& outputs,
		std::string const& name
	){
		auto const iter = outputs.find(name);
		return iter != outputs.end() ? iter->second : 0;
	}

	template < typename Maker >
	auto make_parameter_value_map(
		std::string_view location,
		Maker const& maker,
		parameter_list const& params
	){
		auto const name = detail::to_std_string(maker.name);
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
					detail::to_std_string(maker.to_text[type]));
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
		(void)location; // Silance GCC ...

		if constexpr(hana::is_a< input_maker_tag, Maker >()){
			return input_make_data(maker, make_output_info(data.inputs,
				detail::to_std_string(maker.name)));
		}else if constexpr(hana::is_a< output_maker_tag, Maker >()){
			return output_make_data(maker, get_use_count(data.outputs,
				detail::to_std_string(maker.name)));
		}else if constexpr(hana::is_a< parameter_maker_tag, Maker >()){
			return parameter_make_data(maker,
				make_parameter_value_map(location, maker, data.parameters));
		}else{
			static_assert(detail::false_c< Maker >,
				"maker is not an iop (this is a bug in disposer!)");
		}
	}

	template < typename IOP_RefList >
	class iops_accessory: public add_log< iops_accessory< IOP_RefList > >{
	public:
		iops_accessory(
			IOP_RefList const& iop_list,
			iop_log&& log_fn
		)noexcept
			: iop_list_(iop_list)
			, log_fn_(std::move(log_fn)) {}


		/// \brief Get const reference to an input-, output- or parameter-object
		///        via its corresponding compile time name
		template < typename Name >
		auto const& operator()(Name const& name)const noexcept{
			using name_t = std::remove_reference_t< Name >;
			static_assert(
				hana::is_a< input_name_tag, name_t > ||
				hana::is_a< output_name_tag, name_t > ||
				hana::is_a< parameter_name_tag, name_t >,
				"name is not an input_name, output_name or parameter_name");

			return detail::extract(iop_list_, name);
		}


		/// \brief Implementation of the log prefix
		void log_prefix(log_key&&, logsys::stdlogb& os)const{
			log_fn_(os);
		}


	private:
		IOP_RefList iop_list_;

		/// \brief Reference to an iop_log object
		iop_log log_fn_;
	};

	template < typename IOP_RefList, typename MakeData >
	struct iops_make_data{
		iops_make_data(
			MakeData&& make_data,
			std::string_view location,
			IOP_RefList const& iop_list
		)noexcept
			: data(static_cast< MakeData&& >(make_data))
			, accessory(iop_list, iop_log{
				location, make_data.log_name,
				detail::to_std_string_view(make_data.maker.name)}) {}

		MakeData data;
		iops_accessory< IOP_RefList > accessory;
	};


}


#endif
