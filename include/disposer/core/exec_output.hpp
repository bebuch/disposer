//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__exec_output__hpp_INCLUDED_
#define _disposer__core__exec_output__hpp_INCLUDED_

#include "exec_output_base.hpp"
#include "output.hpp"
#include "config_fn.hpp"

#include "../tool/to_std_string_view.hpp"

#include <io_tools/make_string.hpp>

#include <functional>
#include <variant>


namespace disposer{


	/// \brief The output type while exec
	template < typename Name, typename T >
	class exec_output: public exec_output_base{
	public:
		/// \brief Compile time name of the output
		using name_type = typename output_type::name_type;

		/// \brief Name object
		static constexpr auto name = name_type{};


		/// \brief Constructor
		exec_output(std::size_t use_count)noexcept
			: exec_output_base(use_count) {}


		/// \brief Add given data to \ref data_
		template < typename ... Args >
		void emplace(Args&& args){
			data_.emplace_back(static_cast< Args&& >(args) ...);
		}

		/// \brief Add given data to \ref data_
		template < typename Arg >
		void push(Arg&& value){
			data_.push_back(static_cast< Arg&& >(value));
		}


		/// \brief Get a view to the data
		input_data_r< T > references(exec_input_key&&)const{
			return data_;
		}

		/// \brief Get a reference to the data
		input_data_v< T > values(exec_input_key&&)const{
			if(is_last_use()){
				return std::move(data_);
			}else{
				return data_;
			}
		}

		/// \brief Remove data on last cleanup call
		void cleanup(exec_input_key&&)noexcept{
			if(exec_output_base::cleanup()) data_.clear();
		}


	private:
		/// \brief Putted data of the output
		std::vector< T > data_;
	};


}


#endif
