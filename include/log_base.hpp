//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_log_base_hpp_INCLUDED_
#define _disposer_log_base_hpp_INCLUDED_

#include <ostream>
#include <memory>


namespace disposer{


	class log_base{
	public:
		static std::function< std::unique_ptr< log_base >() > factory;

		virtual ~log_base(){}

		virtual void pre(){}
		virtual void post(){}
		virtual void failed(){}
		virtual void set_exception(std::exception const& error){}
		virtual void unknown_exception(){}
		virtual void have_body(){}
		virtual void exec()const{}

		template < typename T >
		friend log_base& operator<<(log_base& log, T&& data){
			log.os() << static_cast< T&& >(data);
			return log;
		}

	protected:
		virtual std::ostream& os() = 0;
	};


}

#endif
