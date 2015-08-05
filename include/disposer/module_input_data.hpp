//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_module_input_data_hpp_INCLUDED_
#define _disposer_module_input_data_hpp_INCLUDED_

#include "module_output_data.hpp"

#include <stdexcept>


namespace disposer{


	template < typename T >
	class module_input_data{
	public:
		module_input_data(module_output_data_ptr< T > const& data, bool last_use):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return data_->data();
		}

		/// \brief Move out the pointer if last use or get a deep copy
		module_output_data_ptr< T > get(){
			if(last_use_){
				return std::move(data_);
			}else{
				return hana::if_(hana::traits::is_copy_constructible(hana::type< T >),
					[](auto& data){ return std::make_shared< module_output_data< T > >(data); },
					[](auto&)->module_output_data_ptr< T >{ throw std::logic_error("Type '" + boost::typeindex::type_id< T >().pretty_name() + "' is not copy constructible"); }
				)(data());
			}
		}


	private:
		module_output_data_ptr< T > data_;
		bool last_use_;
	};


	template < typename T >
	class module_input_data< std::future< T > >{
	public:
		module_input_data(module_output_data_ptr< std::future< T > > const& data, bool last_use):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return data_->data();
		}

		/// \brief Move out the pointer if last use or get a deep copy
		module_output_data_ptr< T > get(){
			if(last_use_){
				return data_->get();
			}else{
				return hana::if_(hana::traits::is_copy_constructible(hana::type< T >),
					[](auto& data){ return std::make_shared< module_output_data< T > >(data); },
					[](auto&)->module_output_data_ptr< T >{ throw std::logic_error("Type '" + boost::typeindex::type_id< T >().pretty_name() + "' is not copy constructible"); }
				)(data());
			}
		}


	private:
		module_output_data_ptr< std::future< T > > data_;
		bool last_use_;
	};


	template <>
	class module_input_data< std::future< void > >{
	public:
		module_input_data(module_output_data_ptr< std::future< void > > const& data, bool):
			data_(data)
			{}

		void data()const{
			data_->wait();
		}

		void get(){
			data_->wait();
		}

		void wait()const{
			data_->wait();
		}


	private:
		module_output_data_ptr< std::future< void > > data_;
	};


}


#endif
