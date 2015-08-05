//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_output_hpp_INCLUDED_
#define _disposer_output_hpp_INCLUDED_

#include "input.hpp"

#include <vector>


namespace disposer{


	namespace impl{ namespace output{


		class signal_t{
		public:
			void operator()(std::size_t id, input::any_type const& data, type_index const& type)const{
				for(auto& target: targets_){
					target.first.add(id, data, type, target.second);
				}
			}

		private:
			void connect(input::module_input_base& input, bool last_use){
				targets_.emplace_back(input, last_use);
			}

			std::vector< std::pair< input::module_input_base&, bool > > targets_;

			friend struct output_entry;
		};

		struct output_entry{
			signal_t& signal;
			std::vector< type_index > const& types;

			void connect(input::module_input_base& input, bool last_use){
				signal.connect(input, last_use);
			}
		};


	} }


	template < typename T, typename ... U >
	auto make_type_map(){
		return hana::make_map(
			hana::make_pair(hana::type< T >, false),
			hana::make_pair(hana::type< U >, false) ...
		);
	}

	template < typename T, typename ... U >
	struct type_t{
		decltype(make_type_map< T, U ... >()) map = make_type_map< T, U ... >();
	};


	using output_list = std::unordered_map< std::string, impl::output::output_entry >;

	template < typename ... Outputs >
	output_list make_output_list(Outputs& ... outputs);


	namespace impl{ namespace output{


		template < typename T >
		class module_output_interface{
		public:
			using value_type = T;


			module_output_interface(signal_t& signal, type_index&& type): signal_(signal), type_(std::move(type)) {}


			void operator()(std::size_t id, value_type&& value){
				trigger_signal(id, std::make_shared< value_type >(std::move(value)));
			}

			void operator()(std::size_t id, value_type const& value){
				trigger_signal(id, std::make_shared< value_type >(value));
			}

			void operator()(std::size_t id, std::shared_ptr< value_type >&& value){
				trigger_signal(id, std::move(value));
			}

			void operator()(std::size_t id, std::shared_ptr< value_type > const& value){
				trigger_signal(id, value);
			}


		private:
			signal_t& signal_;
			type_index type_;

			void trigger_signal(std::size_t id, std::shared_ptr< value_type > const& value){
				signal_(id, reinterpret_cast< impl::input::any_type const& >(value), type_);
			}
		};


		template < typename T >
		class module_output_interface< std::future< T > >{
		public:
			using value_type = std::future< T >;


			module_output_interface(signal_t& signal, type_index&& type): signal_(signal), type_(std::move(type)) {}


			void operator()(std::size_t id, value_type&& value){
				auto val = std::make_shared< impl::input::future_type< T > >(std::move(value));
				signal_(id, reinterpret_cast< impl::input::any_type const& >(val), type_);
			}


		private:
			signal_t& signal_;
			type_index type_;
		};


		template < typename T, typename ... U >
		class module_output{
		public:
			static constexpr auto value_types = hana::tuple_t< T, U ... >;

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_const), false, std::logical_or<>()),
				"module_output types are not allowed to be const"
			);

			static_assert(
				!hana::fold(hana::transform(value_types, hana::traits::is_reference), false, std::logical_or<>()),
				"module_output types are not allowed to be references"
			);

	// 		static_assert(
	// 			hana::unique(hana::tuple_t< T, U ... >),
	// 			"module_output must have distict types"
	// 		);


			module_output(std::string const& name): name(name) {}

			module_output(module_output const&) = delete;
			module_output(module_output&&) = delete;

			module_output& operator=(module_output const&) = delete;
			module_output& operator=(module_output&&) = delete;


			std::string const name;


			template < typename V, typename W >
			auto put(std::size_t id, W&& value){
				static_assert(hana::contains(value_types, hana::type< V >), "type V in put< V > is not a module_output type");

				if(type_.map[hana::type< V >]){
					throw std::logic_error(
						"module_output '" + name + "' put inactive type '" + type_id_with_cvr< V >().pretty_name() + "'"
					);
				}

				return module_output_interface< V >(signal_, type_id_with_cvr< V >())(id, static_cast< W&& >(value));
			}


		private:
			signal_t signal_;

			std::vector< type_index > active_types_;

			type_t< T, U ... > type_;

			template < typename ... Outputs >
			friend output_list disposer::make_output_list(Outputs& ... outputs);
		};


	} }


	template < typename T, typename ... U >
	class module_output: public impl::output::module_output< T, U ... >{
	public:
		using impl::output::module_output< T, U ... >::module_output;
	};

	template < typename T >
	class module_output< T >: public impl::output::module_output< T >{
	public:
		using impl::output::module_output< T >::module_output;

		template < typename W >
		auto put(std::size_t id, W&& value){
			impl::output::module_output< T >::template put< T >(id, static_cast< W&& >(value));
		}
	};


	template < typename ... Outputs >
	output_list make_output_list(Outputs& ... outputs){
		output_list result({
			{                 // initializer_list
				outputs.name, // name as string
				impl::output::output_entry{
					outputs.signal_, // reference to signal object
					outputs.active_types_
				}
			} ...
		}, sizeof...(Outputs));

		if(result.size() < sizeof...(Outputs)){
			throw std::logic_error("duplicate output variable name");
		}

		return result;
	}


}


#endif
