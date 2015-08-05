//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_output_data_hpp_INCLUDED_
#define _disposer_output_data_hpp_INCLUDED_

#include <memory>
#include <future>
#include <mutex>


namespace disposer{


	template < typename T >
	class output_data;

	template < typename T >
	using output_data_ptr = std::shared_ptr< output_data< T > >;


	template < typename T >
	class output_data{
	public:
		output_data(T&& data): data_(std::move(data)) {}
		output_data(T const& data): data_(data) {}

		output_data_ptr< T > get(){
			return std::make_shared< output_data< T > >(std::move(data_));
		}

		T& data(){
			return data_;
		}

		T const& data()const{
			return data_;
		}

	public:
		T data_;
	};

	template < typename T >
	class output_data< std::future< T > >{
	public:
		output_data(std::future< T >&& future): future_(std::move(future)), called_(false) {}

		output_data_ptr< T > get(){
			get_future();
			return std::make_shared< output_data< T > >(std::move(data_));
		}

		T& data(){
			get_future();
			return data_;
		}

		T const& data()const{
			get_future();
			return data_;
		}


	private:
		void get_future()const{
			std::lock_guard< std::mutex > lock(mutex_);

			if(called_) return;

			data_ = future_.get();
			called_ = true;
		}

		std::future< T > mutable future_;
		T mutable data_;
		std::mutex mutable mutex_;
		bool mutable called_;
	};

	template <>
	class output_data< std::future< void > >{
	public:
		output_data(std::future< void >&& future): future_(std::move(future)), called_(false) {}

		void wait()const{
			std::lock_guard< std::mutex > lock(mutex_);

			if(called_) return;

			future_.get();
			called_ = true;
		}


	private:
		std::future< void > mutable future_;
		std::mutex mutable mutex_;
		bool mutable called_;
	};


	template < typename T >
	using output_data_ptr = std::shared_ptr< output_data< T > >;


}


#endif
