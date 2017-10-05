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

#include "../tool/input_data.hpp"
#include "../tool/to_std_string_view.hpp"

#include <io_tools/make_string.hpp>

#include <boost/hana/tuple.hpp>

#include <functional>
#include <variant>


namespace disposer{


	/// \brief The output type while exec
	template < typename T >
	class unnamed_exec_output: public exec_output_base{
	public:
		using exec_output_base::exec_output_base;


		/// \brief Add given data to \ref data_
		template < typename ... Args >
		void emplace(Args&& ... args){
			data_.emplace_back(static_cast< Args&& >(args) ...);
		}

		/// \brief Add given data to \ref data_
		template < typename Arg >
		void push(Arg&& value){
			data_.push_back(static_cast< Arg&& >(value));
		}


		/// \brief Get a view to the data
		input_data_r< T > references()const{
			return data_;
		}

		/// \brief Get a reference to the data
		input_data_v< T > values()const{
			if(is_last_use()){
				return std::move(data_);
			}else{
				return data_;
			}
		}

		/// \brief Remove data on last cleanup call
		void cleanup()noexcept{
			if(is_cleanup()) data_.clear();
		}


	private:
		/// \brief Putted data of the output
		std::vector< T > data_;
	};


	/// \brief The output type while exec
	template < typename Name, typename T >
	class exec_output: public unnamed_exec_output< T >{
	public:
		/// \brief Constructor
		exec_output(
			hana::tuple< output< Name, T >&, output_map_type& > const& data
		)noexcept
			: unnamed_exec_output< T >(data[hana::size_c< 0 >].use_count())
		{
			data[hana::size_c< 1 >].emplace(&data[hana::size_c< 0 >], this);
		}

		/// \brief Compile time name of the output
		using name_type = Name;

		/// \brief Name object
		static constexpr auto name = name_type{};

		using unnamed_exec_output< T >::unnamed_exec_output;
	};


}


#endif
