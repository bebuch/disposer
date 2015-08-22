//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__input_base_hpp_INCLUDED_
#define _disposer__input_base_hpp_INCLUDED_

#include <boost/type_index.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>


namespace disposer{


	using boost::typeindex::type_index;


	struct any_type;

	class input_base{
	public:
		input_base(std::string const& name): name(name) {}

		input_base(input_base const&) = delete;
		input_base(input_base&&) = delete;

		input_base& operator=(input_base const&) = delete;
		input_base& operator=(input_base&&) = delete;


		virtual ~input_base() = default;


		virtual void add(std::size_t id, any_type const& value, type_index const& type, bool last_use) = 0;
		virtual void cleanup(std::size_t id)noexcept = 0;
		virtual bool activate_types(std::vector< type_index > const& types) noexcept = 0;
		virtual std::vector< type_index > types()const = 0;


		std::string const name;
	};


	using input_list = std::unordered_map< std::string, input_base& >;

	template < typename ... Inputs >
	input_list make_input_list(Inputs& ... inputs){
		input_list result({
			{                // initializer_list
				inputs.name, // name as string;
				inputs       // reference to object
			} ...
		}, sizeof...(Inputs));

		if(result.size() < sizeof...(Inputs)){
			throw std::logic_error("duplicate output variable name");
		}

		return result;
	}


}


#endif
