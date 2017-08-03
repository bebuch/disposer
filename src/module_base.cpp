//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/module_base.hpp>


namespace disposer{


	/// \brief Class module_base access key
	struct module_base_key{
	private:
		/// \brief Constructor
		constexpr module_base_key()noexcept = default;
		friend class module_base;
	};


	module_base::module_base(
		std::string const& type_name,
		std::string const& chain,
		std::size_t number,
		input_list&& inputs,
		output_list&& outputs
	):
		type_name(type_name),
		chain(chain),
		number(number),
		id(id_),
		id_(0),
		inputs_(std::move(inputs)),
		outputs_(std::move(outputs))
		{}


	void module_base::cleanup(chain_key&&, std::size_t id)noexcept{
		for(auto& input: inputs_){
			input.get().cleanup(module_base_key(), id);
		}
	}

	void module_base::set_id(chain_key&&, std::size_t id){
		id_ = id;
		for(auto& input: inputs_){
			input.get().set_id(module_base_key(), id);
		}
		for(auto& output: outputs_){
			output.get().prepare(module_base_key(), id);
		}
	}

	std::map< std::string, output_base* >
	module_base::get_outputs(creator_key&&)const{
		std::map< std::string, output_base* > map;
		for(auto output: outputs_){
			map.emplace(output.get().get_name(), &output.get());
		}
		return map;
	}



}
