//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__input_data__hpp_INCLUDED_
#define _disposer__input_data__hpp_INCLUDED_

#include "output_data.hpp"
#include "type_name.hpp"

#include <stdexcept>


namespace disposer{


	/// \brief Wrapper for input data
	template < typename T >
	class input_data{
	public:
		/// \brief Type of the data
		using value_type = T;

		/// \brief Constructor
		input_data(output_data_ptr< T > const& data, bool last_use):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return data_->data();
		}

		/// \brief If last use move out the pointer, otherwise get a deep copy
		output_data_ptr< T > get(){
			if(last_use_){
				return std::move(data_);
			}else{
				if constexpr(std::is_copy_constructible_v< T >){
					return std::make_shared< output_data< T > >(data());
				}else{
					throw std::logic_error(
						"Type [" + type_name< T >()
						+ "] is not copy constructible"
					);
				}
			}
		}


	private:
		/// \brief shared_ptr to the data
		output_data_ptr< T > data_;

		/// \brief Flag if this is the last use of the data in the chain
		bool last_use_;
	};


	///\brief Specialization for std::future with data
	template < typename T >
	class input_data< std::future< T > >{
	public:
		/// \brief Type of the data
		using value_type = std::future< T >;

		/// \brief Constructor
		input_data(
			output_data_ptr< std::future< T > > const& data, bool last_use
		):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return data_->data();
		}

		/// \brief Move out the pointer if last use or get a deep copy
		output_data_ptr< T > get(){
			if(last_use_){
				return data_->get();
			}else{
				if constexpr(std::is_copy_constructible_v< T >){
					return std::make_shared< output_data< T > >(data());
				}else{
					throw std::logic_error(
						"Type '" + type_name< T >()
						+ "' is not copy constructible"
					);
				}
			}
		}


	private:
		/// \brief shared_ptr to the data
		output_data_ptr< std::future< T > > data_;
		bool last_use_;
	};


	///\brief Specialization for std::future without data
	template <>
	class input_data< std::future< void > >{
	public:
		/// \brief Type of the data
		using value_type = std::future< void >;

		/// \brief Constructor
		input_data(output_data_ptr< std::future< void > > const& data, bool):
			data_(data)
			{}

		/// \brief Block until future is ready
		void data()const{
			data_->wait();
		}

		/// \brief Block until future is ready
		void get(){
			data_->wait();
		}

		/// \brief Block until future is ready
		void wait()const{
			data_->wait();
		}


	private:
		/// \brief shared_ptr to the data
		output_data_ptr< std::future< void > > data_;
	};


}


#endif
