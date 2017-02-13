//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__output_data__hpp_INCLUDED_
#define _disposer__output_data__hpp_INCLUDED_

#include <memory>
#include <future>
#include <mutex>


namespace disposer{


	template < typename T >
	class output_data;

	/// \brief std::shared_ptr of output_data
	template < typename T >
	using output_data_ptr = std::shared_ptr< output_data< T > >;


	/// \brief Wrapper for output data
	template < typename T >
	class output_data{
	public:
		/// \brief Constructor
		output_data(T&& data): data_(std::move(data)) {}

		/// \brief Constructor
		output_data(T const& data): data_(data) {}


		/// \brief Move data into a new shared_ptr and get it
		output_data_ptr< T > get(){
			return std::make_shared< output_data< T > >(std::move(data_));
		}

		/// \brief Get reference to data
		T& data(){
			return data_;
		}

		/// \brief Get const reference to data
		T const& data()const{
			return data_;
		}


	public:
		/// \brief The data
		T data_;
	};


	///\brief Specialization for std::future with data
	template < typename T >
	class output_data< std::future< T > >{
	public:
		/// \brief Constructor
		output_data(std::future< T >&& future):
			future_(std::move(future)), called_(false) {}

		output_data_ptr< T > get(){
			get_future();
			return std::make_shared< output_data< T > >(std::move(data_));
		}

		/// \brief Block until future is ready and get reference to data
		T& data(){
			get_future();
			return data_;
		}

		/// \brief Block until future is ready and get const reference to data
		T const& data()const{
			get_future();
			return data_;
		}


	private:
		/// \brief Block until future is ready
		void get_future()const{
			std::lock_guard< std::mutex > lock(mutex_);

			if(called_) return;

			data_ = future_.get();
			called_ = true;
		}


		/// \brief The future
		std::future< T > mutable future_;

		/// \brief The data from future
		T mutable data_;

		/// \brief A mutex
		std::mutex mutable mutex_;

		/// \brief Flag if data is available
		bool mutable called_;
	};


	///\brief Specialization for std::future without data
	template <>
	class output_data< std::future< void > >{
	public:
		/// \brief Constructor
		output_data(std::future< void >&& future):
			future_(std::move(future)), called_(false) {}

		/// \brief Block until future is ready
		void wait()const{
			std::lock_guard< std::mutex > lock(mutex_);

			if(called_) return;

			future_.get();
			called_ = true;
		}


	private:
		/// \brief The future
		std::future< void > mutable future_;

		/// \brief The data from future
		std::mutex mutable mutex_;

		/// \brief Flag if data is available
		bool mutable called_;
	};


}


#endif
