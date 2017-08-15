//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__input_exec__hpp_INCLUDED_
#define _disposer__core__input_exec__hpp_INCLUDED_

#include "input_exec_base.hpp"
#include "output_exec.hpp"

#include "../tool/input_data.hpp"


namespace disposer{


	struct module_exec_key;

	/// \brief Hana Tag for input_exec
	struct input_exec_tag{};

	/// \brief The actual input_exec type
	template < typename Name, typename T, bool IsRequired >
	class input_exec: public input_exec_base{
	public:
		/// \brief Hana tag to identify input_execs
		using hana_tag = input_exec_tag;


		/// \brief Compile time name of the input
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		input_exec(output_exec_base* output)noexcept
			: input_exec_base(output) {}


		/// \brief Is the input connected to an output
		bool is_connected()noexcept{
			if constexpr(IsRequired){
				static_assert(false_c< IsRequired >,
					"Input is required and therefore always connected! "
					"Just don't ask ;-)");
			}

			return output_ptr() != nullptr;
		}

		/// \brief Get all data without transferring ownership
		input_data_r< T > references(){
			verify_connection();
			return output_ptr()->references();
		}

		/// \brief Get all data with transferring ownership
		input_data_v< T > values(){
			verify_connection();
			return output_ptr()->values();
		}

		/// \brief Tell the connected output that this input finished
		void cleanup(module_exec_key&)noexcept{
			if(output_ptr()) output_ptr()->cleanup(input_exec_key());
		}


	private:
		/// \brief Get a pointer to the connected output_exec or a nullptr
		output_exec< T >* output_ptr()const noexcept{
			return static_cast< output_exec< T >* >(
				input_exec_base::output_ptr());
		}

		void verify_connection()noexcept(IsRequired){
			if constexpr(!IsRequired) if(!output_ptr()){
				throw std::logic_error("input(" + to_std_string(name)
					+ ") is not linked to an output");
			}
		}
	};


}


#endif
