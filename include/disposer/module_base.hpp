//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_module_base_hpp_INCLUDED_
#define _disposer_module_base_hpp_INCLUDED_

#include "parameter_processor.hpp"
#include "module_output.hpp"
#include "module_input.hpp"

#include <functional>


namespace disposer{


	struct module_not_as_start: std::logic_error{
		module_not_as_start(std::string const& type, std::string const& chain):
			std::logic_error("module type '" + type + "' can not be used as start of chain '" + chain + "'"){}
	};


	struct module_base{
		module_base(std::string const& type, std::string const& chain, std::string const& name);

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

		std::string const type;
		std::string const chain;
		std::string const name;

		output_list outputs;
		input_list inputs;
	};


	using module_ptr = std::shared_ptr< module_base >;


	using io_list = std::set< std::string >;

	using maker_function = std::function< module_ptr(std::string const& type, std::string const&, std::string const&, io_list const&, io_list const&, parameter_processor&, bool is_start) >;

	void add_module_maker(std::string const& type, maker_function&& function);

	module_ptr make_module(std::string const& type, std::string const& chain, std::string const& name, io_list const& inputs, io_list const& outputs, parameter_processor&& parameters, bool is_start);


}


#endif
