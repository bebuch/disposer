//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_input__hpp_INCLUDED_
#define _disposer__core__exec_input__hpp_INCLUDED_

#include "exec_input_base.hpp"
#include "exec_output.hpp"

#include "../tool/input_data.hpp"


namespace disposer{


	struct exec_module_key;

	/// \brief The actual exec_input type
	template < typename Name, typename T, bool IsRequired >
	class exec_input: public exec_input_base{
	public:
		/// \brief Compile time name of the input
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		exec_input(exec_output_base* output)noexcept
			: exec_input_base(output) {}


		/// \brief Is the input connected to an output
		bool is_connected()noexcept{
			static_assert(IsRequired,
				"Input is required and therefore always connected! "
				"Just don't ask ;-)");

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
		void cleanup(exec_module_key&)noexcept{
			if(output_ptr()) output_ptr()->cleanup(exec_input_key());
		}


	private:
		/// \brief Get a pointer to the connected exec_output or a nullptr
		unnamed_exec_output< T >* output_ptr()const noexcept{
			return static_cast< unnamed_exec_output< T >* >(
				exec_input_base::output_ptr());
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
