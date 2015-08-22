//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_base_hpp_INCLUDED_
#define _disposer__module_base_hpp_INCLUDED_

#include "make_data.hpp"
#include "output_base.hpp"
#include "input_base.hpp"

#include <functional>


namespace disposer{


	struct module_not_as_start: std::logic_error{
		module_not_as_start(make_data const& data):
			std::logic_error("module type '" + data.type_name + "' can not be used as start of chain '" + data.chain + "'"){}
	};


	struct module_base{
		module_base(make_data const& data);

		module_base(module_base const&) = delete;
		module_base(module_base&&) = delete;

		module_base& operator=(module_base const&) = delete;
		module_base& operator=(module_base&&) = delete;

		virtual ~module_base() = default;

		virtual void trigger(std::size_t id) = 0;

		void cleanup(std::size_t id)noexcept{
			for(auto& input: inputs){
				input.second.cleanup(id);
			}
		}

		std::string const type_name;
		std::string const chain;
		std::string const name;
		std::size_t const number;

		output_list outputs;
		input_list inputs;
	};


	using module_ptr = std::unique_ptr< module_base >;


}


#endif
