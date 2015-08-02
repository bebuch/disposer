//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_input_hpp_INCLUDED_
#define _disposer_input_hpp_INCLUDED_

#include <boost/type_index.hpp>
#include <boost/hana.hpp>
#include <boost/hana/ext/std/type_traits.hpp>

#include <unordered_map>
#include <future>
#include <string>
#include <mutex>


namespace disposer{


	namespace impl{ namespace input{


		struct any_type;

		struct module_input_base{
			virtual ~module_input_base() = default;

			virtual void add(std::size_t id, any_type const& value, bool last_use) = 0;
			virtual void cleanup(std::size_t id)noexcept = 0;
		};

		struct input_entry{
			module_input_base& object;
			boost::typeindex::type_index const type;
		};

		template < typename T >
		class future_type{
		public:
			future_type(std::future< T >&& future): future_(std::move(future)), called_(false) {}

			future_type(future_type const&) = delete;
			future_type(future_type&&) = delete;

			future_type& operator=(future_type const&) = delete;
			future_type& operator=(future_type&&) = delete;

			std::shared_ptr< T > get(){
				get_future();
				return std::move(data_);
			}

			T& data()const{
				get_future();
				return *data_;
			}


		private:
			void get_future()const{
				if(!called_.exchange(true)){
					data_ = std::make_shared< T >(future_.get());
				}
			}

			std::future< T > mutable future_;
			std::shared_ptr< T > mutable data_;
			std::atomic< bool > mutable called_;
		};

		template <>
		class future_type< void >{
		public:
			future_type(std::future< void >&& future): future_(std::move(future)), called_(false) {}

			future_type(future_type const&) = delete;
			future_type(future_type&&) = delete;

			future_type& operator=(future_type const&) = delete;
			future_type& operator=(future_type&&) = delete;

			void get(){
				if(!called_.exchange(true)) future_.get();
			}

			void wait()const{
				future_.wait();
			}


		private:
			std::future< void > future_;
			std::atomic< bool > called_;
		};


	} }


	using input_list = std::unordered_map< std::string, impl::input::input_entry >;


	template < typename T >
	class input_data{
	public:
		input_data(std::shared_ptr< T > const& data, bool last_use):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return *data_;
		}

		/// \brief Move out the pointer if last use or get a deep copy
		std::shared_ptr< T > get(){
			if(last_use_){
				return std::move(data_);
			}else{
				return boost::hana::if_(boost::hana::traits::is_copy_constructible(boost::hana::type< T >),
					[](auto& data){ return std::make_shared< T >(data); },
					[](auto&)->std::shared_ptr< T >{ throw std::logic_error("Type '" + boost::typeindex::type_id< T >().pretty_name() + "' is not copy constructible"); }
				)(data());
			}
		}


	private:
		std::shared_ptr< T > data_;
		bool last_use_;
	};


	template < typename T >
	class input_data< std::future< T > >{
	public:
		input_data(std::shared_ptr< impl::input::future_type< T > > const& data, bool last_use):
			data_(data),
			last_use_(last_use)
			{}

		/// \brief Access the data via const reference
		T const& data()const{
			return data_->data();
		}

		/// \brief Move out the pointer if last use or get a deep copy
		std::shared_ptr< T > get(){
			if(last_use_){
				return std::move(data_->get());
			}else{
				return boost::hana::if_(boost::hana::traits::is_copy_constructible(boost::hana::type< T >),
					[](auto& data){ return std::make_shared< T >(data); },
					[](auto&)->std::shared_ptr< T >{ throw std::logic_error("Type '" + boost::typeindex::type_id< T >().pretty_name() + "' is not copy constructible"); }
				)(data());
			}
		}


	private:
		std::shared_ptr< impl::input::future_type< T > > data_;
		bool last_use_;
	};


	template <>
	class input_data< std::future< void > >{
	public:
		input_data(std::shared_ptr< impl::input::future_type< void > > const& data, bool):
			data_(data)
			{}

		void get(){
			data_->get();
		}

		void wait()const{
			data_->wait();
		}


	private:
		std::shared_ptr< impl::input::future_type< void > > data_;
	};


	template < typename T >
	class module_input: public impl::input::module_input_base{
	public:
		using value_type = T;

		module_input(std::string const& name): name(name) {}

		module_input(module_input const&) = delete;
		module_input(module_input&&) = delete;

		module_input& operator=(module_input const&) = delete;
		module_input& operator=(module_input&&) = delete;

		std::string const name;

		virtual void add(std::size_t id, impl::input::any_type const& value, bool last_use)override{
			auto data = reinterpret_cast< std::shared_ptr< T > const& >(value);

			std::lock_guard< std::mutex > lock(mutex_);
			data_.emplace(id, input_data< T >(data, last_use));
		}

		virtual void cleanup(std::size_t id)noexcept override{
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);
			data_.erase(from, to);
		}

		std::multimap< std::size_t, input_data< T > > get(std::size_t id){
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);

			std::multimap< std::size_t, input_data< T > > result(std::make_move_iterator(from), std::make_move_iterator(to));
			data_.erase(from, to);

			return result;
		}

	private:
		std::mutex mutex_;

		std::multimap< std::size_t, input_data< T > > data_;
	};


	template < typename T >
	class module_input< std::future< T > >: public impl::input::module_input_base{
	public:
		using value_type = std::future< T >;

		module_input(std::string const& name): name(name) {}

		module_input(module_input const&) = delete;
		module_input(module_input&&) = delete;

		module_input& operator=(module_input const&) = delete;
		module_input& operator=(module_input&&) = delete;

		std::string const name;

		virtual void add(std::size_t id, impl::input::any_type const& value, bool last_use)override{
			auto data = reinterpret_cast< std::shared_ptr< impl::input::future_type< T > > const& >(value);

			std::lock_guard< std::mutex > lock(mutex_);
			data_.emplace(id, input_data< std::future< T > >(data, last_use));
		}

		virtual void cleanup(std::size_t id)noexcept override{
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);
			data_.erase(from, to);
		}

		std::multimap< std::size_t, input_data< std::future< T > > > get(std::size_t id){
			std::lock_guard< std::mutex > lock(mutex_);
			auto from = data_.begin();
			auto to = data_.upper_bound(id);

			std::multimap< std::size_t, input_data< std::future< T > > > result(std::make_move_iterator(from), std::make_move_iterator(to));
			data_.erase(from, to);

			return result;
		}

	private:
		std::mutex mutex_;

		std::multimap< std::size_t, input_data< std::future< T > > > data_;
	};


	template < typename ... Inputs >
	input_list make_input_list(Inputs& ... inputs){
		input_list result({
			{                // initializer_list
				inputs.name, // name as string
				impl::input::input_entry{
					inputs,
					boost::typeindex::type_id_with_cvr< typename Inputs::value_type >()
				}
			} ...
		}, sizeof...(Inputs));

		if(result.size() < sizeof...(Inputs)){
			throw std::logic_error("duplicate output variable name");
		}

		return result;
	}


}


#endif
