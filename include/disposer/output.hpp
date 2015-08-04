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
					target.first.object.add(id, data, type, target.second);
				}
			}

		private:
			void connect(input::input_entry& input, bool last_use){
				targets_.emplace_back(input, last_use);
			}

			std::vector< std::pair< input::input_entry&, bool > > targets_;

			friend struct output_entry;
		};

		struct output_entry{
			signal_t& signal;
			type_index const& type;

			void connect(input::input_entry& input, bool last_use){
				signal.connect(input, last_use);
			}
		};


	} }


	using output_list = std::unordered_map< std::string, impl::output::output_entry >;

	template < typename ... Outputs >
	output_list make_output_list(Outputs& ... outputs);


	namespace impl{ namespace output{


		template < typename T >
		class module_output_interface{
		public:
			using value_type = T;


			module_output_interface(signal_t& signal, type_index const& type): signal(signal), type(type) {}


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
			signal_t& signal;
			type_index const& type;

			void trigger_signal(std::size_t id, std::shared_ptr< value_type > const& value){
				signal(id, reinterpret_cast< impl::input::any_type const& >(value), type);
			}
		};


		template < typename T >
		class module_output_interface< std::future< T > >{
		public:
			using value_type = std::future< T >;


			module_output_interface(signal_t& signal, type_index const& type): signal(signal), type(type) {}


			void operator()(std::size_t id, value_type&& value){
				auto val = std::make_shared< impl::input::future_type< T > >(std::move(value));
				signal(id, reinterpret_cast< impl::input::any_type const& >(val), type);
			}


		private:
			signal_t& signal;
			type_index const& type;
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

				if(type != type_id_with_cvr< V >()){
					throw std::logic_error(
						"module_output '" + name + "' put type '" +
						type_id_with_cvr< V >().pretty_name() +
						"', but active type is '" + type.pretty_name() + "'"
					);
				}

				return module_output_interface< V >(signal, type)(id, static_cast< W&& >(value));
			}


		private:
			signal_t signal;
			type_index type;

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
					outputs.signal, // reference to signal object
					outputs.type
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
