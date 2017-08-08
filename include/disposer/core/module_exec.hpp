//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__module_exec__hpp_INCLUDED_
#define _disposer__core__module_exec__hpp_INCLUDED_

#include "module.hpp"

#include "../tool/exec_list_t.hpp"

#include <cassert>


namespace disposer{


	/// \brief The actual module_exec type
	template < typename List, typename StateMakerFn, typename ExecFn >
	class module_exec: public module_exec_base{
	public:
		/// \brief Constructor
		module_exec(
			module< List, StateMakerFn, ExecFn >& module,
			std::size_t id,
			output_map_type& output_map
		)noexcept
			: module_(module)
			, id_(id) {}


		/// \brief Name of the process chain in config file section 'chain'
		std::string const& chain()noexcept const{
			return module_.chain();
		}

		/// \brief Name of the module type given via class module_declarant
		std::string const& type_name()noexcept const{
			return module_.type_name();
		}

		/// \brief Position of the module in the process chain
		std::size_t number()noexcept const{
			return module_.number();
		}

		/// \brief Current exec id
		std::size_t id()noexcept const{
			return id_;
		}


	private:
		/// \brief Reference to the module
		module< List, StateMakerFn, ExecFn >& module_;

		/// \brief Current exec id
		std::size_t const id_;

		/// \brief List of input_exec's and output_exec's
		module_exec_data< exec_list_t< List > > data_;


		/// \brief The actual worker function called one times per trigger
		virtual void exec()override{
			assert(state_);
			exec_fn_(accessory_, *state_);
		}

		/// \brief The cleanup function
		virtual void cleanup()noexcept override{
			for(auto& input: inputs_){
				input.get().cleanup(module_base_key());
			}
		}

// 		/// \brief Get map from output names to output_exec_base pointers
// 		virtual output_map_type get_output_map()override const{
// 			output_map_type map;
// 			for(auto output: outputs_){
// 				map.emplace(output.get().get_name(), &output.get());
// 			}
// 			return map;
// 		}
	};


}


#endif
