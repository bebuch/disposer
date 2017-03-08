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

#include "input_base.hpp"
#include "output_base.hpp"
#include "output_data.hpp"
#include "unpack_to.hpp"
#include "type_name.hpp"

#include <io_tools/make_string.hpp>

#include <functional>


namespace disposer{


	template < typename T >
	class output_interface{
	public:
		using value_type = T;


		output_interface(signal_t& signal): signal_(signal) {}


		void operator()(std::size_t id, value_type&& value){
			exec_signal(
				id,
				std::make_shared< output_data< value_type > >(std::move(value))
			);
		}

		void operator()(std::size_t id, value_type const& value){
			exec_signal(
				id,
				std::make_shared< output_data< value_type > >(value)
			);
		}

		void operator()(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			exec_signal(id, value);
		}


	private:
		signal_t& signal_;

		void exec_signal(
			std::size_t id, output_data_ptr< value_type > const& value
		){
			signal_(
				id,
				reinterpret_cast< any_type const& >(value),
				type_index::type_id_with_cvr< T >()
			);
		}
	};


	template < typename String, typename ... T >
	class output: public output_base{
	public:
		static_assert(hana::is_a< hana::string_tag, String >);

		static constexpr auto types = hana::make_set(hana::type_c< T > ...);

		static constexpr std::size_t type_count = sizeof...(T);


		static_assert(type_count != 0,
			"disposer::output needs at least on type");

		static_assert(!hana::any_of(types, hana::traits::is_const),
			"disposer::output types are not allowed to be const");

		static_assert(!hana::any_of(types, hana::traits::is_reference),
			"disposer::output types are not allowed to be references");


		using output_base::output_base;

		output(): output_base(String::c_str()) {}


		template < typename V, typename W >
		void put(W&& value){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in put< V > is not a output type"
			);

			if(!enabled_types_[type_index::type_id_with_cvr< V >()]){
				using namespace std::literals::string_literals;
				throw std::logic_error(io_tools::make_string(
					"output '", name, "' put disabled type [",
					type_name_with_cvr< V >(), "]"
				));
			}

			output_interface< V >{signal}(id, static_cast< W&& >(value));
		}


		template < typename V >
		void enable(){
			static_assert(
				hana::contains(types, hana::type_c< V >),
				"type V in enable< V > is not a output type"
			);

			enabled_types_[type_index::type_id_with_cvr< V >()] = true;
		}

		template < typename V, typename W, typename ... X >
		void enable(){
			enable< V >();
			enable< W, X ... >();
		}

		void enable_types(std::vector< type_index > const& types){
			for(auto& type: types){
				auto iter = std::find(
					enabled_types_.begin(),
					enabled_types_.end(),
					type
				);

				if(iter == enabled_types_.end()){
					throw std::logic_error(io_tools::make_string(
						"type [", type.pretty_name(),
						"] is not an output type of '", name, "'"
					));
				}

				iter->second = true;
			}
		}


		static constexpr std::string_view name{String::c_str()};


	protected:
		std::vector< type_index > enabled_types()const override{
			std::vector< type_index > result;
			result.reserve(type_count);
			for(auto const& [type, enabled]: enabled_types_){
				if(enabled) result.push_back(type);
			}
			return result;
		}


	private:
		std::unordered_map< type_index, bool > enabled_types_{
				{type_index::type_id_with_cvr< T >(), false} ...
			};
	};


}


namespace disposer::interface::module{


// 	template < typename ... OutputPairs >
// 	constexpr void out(OutputPairs&& ... outputs){
// 		static_assert((hana::is_a< hana::pair_tag, OutputPairs > && ...));
// 		static_assert((hana::is_a< hana::string_tag,
// 			decltype(hana::first(outputs)) > && ...));
// 		static_assert((std::is_base_of_v< output_base,
// 			decltype(+hana::second(outputs)) > && ...));
// 		return hana::make_map(outputs);
// 	}


	template < typename String, typename Types >
	constexpr auto output(String&& name, Types&& types){
		using namespace boost::hana;
		static_assert(hana::is_a< hana::string_tag, String >);

		if constexpr(hana::is_a< hana::type_tag, Types >){
			using output_type = ::disposer::output< std::decay_t< String >,
				typename decltype(+types)::type >;

			return hana::make_pair(static_cast< String&& >(name),
				output_type());
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));

			auto string_and_types =
				hana::prepend(hana::to_tuple(types), hana::type_c< String >);

			using output_type = typename decltype(::disposer::unpack_to<
				::disposer::output >(string_and_types))::type;

			return hana::make_pair(
				static_cast< String&& >(name),
				output_type());
		}
	}


}


#endif
