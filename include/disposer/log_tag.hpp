//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__log_tag__hpp_INCLUDED_
#define _disposer__log_tag__hpp_INCLUDED_

#include "log_base.hpp"
#include "time_to_string.hpp"
#include "mask_non_print.hpp"

#include <boost/type_index.hpp>

#include <atomic>


namespace disposer{


	class log_tag_base{
	public:
		static std::size_t unique_id(){
			static std::atomic< std::size_t > next_id(0);
			return next_id++;
		}

		log_tag_base():
			body_(false),
			exception_(false),
			id_(unique_id()),
			start_(std::chrono::system_clock::now())
			{}

		void pre(){
			auto end = std::chrono::system_clock::now();

			os_ << std::setfill('0') << std::setw(6) << id_ << ' ';

			time_to_string(os_, start_);

			if(body_){
				os_ << " ( " << std::setfill(' ') << std::setprecision(3)
					<< std::setw(12)
					<< std::chrono::duration< double, std::milli >(
							end - start_
						).count() << "ms ) ";
			}else{
				os_ << " ( no content     ) ";
			}
		}

		void post(){
			if(exception_) os_ << " (failed)";
			os_ << exception_text_;
		}

		void failed(){
			exception_ = true;
		}

		void set_exception(std::exception const& error){
			exception_text_ = " (exception catched: [" +
				boost::typeindex::type_id_runtime(error).pretty_name() +
				"] " + error.what() + ")";
		}

		void unknown_exception(){
			exception_text_ = " (unknown exception catched)";
		}

		void have_body(){
			body_ = true;
		}

		void exec()const{
			std::clog << (mask_non_print(os_.str()) + '\n');
		}

		template < typename T >
		friend log_tag_base& operator<<(log_tag_base& log, T&& data){
			log.os_ << static_cast< T&& >(data);
			return log;
		}

	protected:
		std::ostringstream os_;
		std::string exception_text_;
		bool body_;
		bool exception_;
		std::size_t id_;
		std::chrono::system_clock::time_point const start_;
	};


	class log_tag: public log_base, protected log_tag_base{
	public:
		void pre()override{
			log_tag_base::pre();
		}

		void post()override{
			log_tag_base::post();
		}

		void failed()override{
			log_tag_base::failed();
		}

		void set_exception(std::exception const& error)override{
			log_tag_base::set_exception(error);
		}

		void unknown_exception()override{
			log_tag_base::unknown_exception();
		}

		void have_body()override{
			log_tag_base::have_body();
		}

		void exec()const override{
			log_tag_base::exec();
		}

		template < typename T >
		friend log_tag& operator<<(log_tag& log, T&& data){
			log.os() << static_cast< T&& >(data);
			return log;
		}


	protected:
		std::ostream& os()override{
			return os_;
		}
	};


}

#endif
