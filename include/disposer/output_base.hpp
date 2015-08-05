//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_output_base_hpp_INCLUDED_
#define _disposer_output_base_hpp_INCLUDED_

#include "input_base.hpp"

#include <utility>


namespace disposer{


	class signal_t{
	public:
		void operator()(std::size_t id, any_type const& data, type_index const& type)const{
			for(auto& target: targets_){
				target.first.add(id, data, type, target.second);
			}
		}

		void connect(input_base& input, bool last_use){
			targets_.emplace_back(input, last_use);
		}

	private:
		std::vector< std::pair< input_base&, bool > > targets_;
	};


	class output_base{
	public:
		output_base(std::string const& name): name(name) {}

		output_base(output_base const&) = delete;
		output_base(output_base&&) = delete;

		output_base& operator=(output_base const&) = delete;
		output_base& operator=(output_base&&) = delete;


		std::string const name;

		signal_t signal;

		std::vector< type_index > active_types;
	};


	using output_list = std::unordered_map< std::string, output_base& >;

	template < typename ... Outputs >
	output_list make_output_list(Outputs& ... outputs){
		output_list result({
			{                 // initializer_list
				outputs.name, // name as string
				outputs       // reference to object
			} ...
		}, sizeof...(Outputs));

		if(result.size() < sizeof...(Outputs)){
			throw std::logic_error("duplicate output variable name");
		}

		return result;
	}


}


#endif
