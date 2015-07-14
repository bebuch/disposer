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
			void operator()(std::size_t id, input::any_type const& data)const{
				for(auto& target: targets_){
					target.first.object.add(id, data, target.second);
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
			boost::typeindex::type_index const type;

			void connect(input::input_entry& input, bool last_use){
				signal.connect(input, last_use);
			}
		};


	} }


	using output_list = std::unordered_map< std::string, impl::output::output_entry >;

	template < typename T >
	class module_output{
	public:
		using value_type = T;

		module_output(std::string const& name): name(name) {}

		module_output(module_output const&) = delete;
		module_output(module_output&&) = delete;

		module_output& operator=(module_output const&) = delete;
		module_output& operator=(module_output&&) = delete;

		std::string const name;

		void operator()(std::size_t id, T&& value){
			trigger_signal(id, std::make_shared< T >(std::move(value)));
		}

		void operator()(std::size_t id, T const& value){
			trigger_signal(id, std::make_shared< T >(value));
		}

		void operator()(std::size_t id, std::shared_ptr< T > const& value){
			trigger_signal(id, value);
		}

	private:
		impl::output::signal_t signal;

		void trigger_signal(std::size_t id, std::shared_ptr< T > const& value){
			signal(id, reinterpret_cast< impl::input::any_type const& >(value));
		}

		template < typename ... Outputs >
		friend output_list make_output_list(Outputs& ... outputs);
	};

	template < typename T >
	class module_output< std::future< T > >{
	public:
		using value_type = std::future< T >;

		module_output(std::string const& name): name(name) {}

		module_output(module_output const&) = delete;
		module_output(module_output&&) = delete;

		module_output& operator=(module_output const&) = delete;
		module_output& operator=(module_output&&) = delete;

		std::string const name;

		void operator()(std::size_t id, std::future< T >&& value){
			auto val = std::make_shared< impl::input::future_type< T > >(std::move(value));
			signal(id, reinterpret_cast< impl::input::any_type const& >(val));
		}

	private:
		impl::output::signal_t signal;

		template < typename ... Outputs >
		friend output_list make_output_list(Outputs& ... outputs);
	};


	template < typename ... Outputs >
	output_list make_output_list(Outputs& ... outputs){
		output_list result({
			{                 // initializer_list
				outputs.name, // name as string
				impl::output::output_entry{
					outputs.signal, // reference to signal object
					boost::typeindex::type_id_with_cvr< typename Outputs::value_type >()
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
