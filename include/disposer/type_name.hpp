//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__type_name__hpp_INCLUDED_
#define _disposer__type_name__hpp_INCLUDED_

#include <boost/type_index.hpp>

#include <string>


namespace disposer{


	template < typename T >
	std::string type_name(T&& value){
		try{
			return boost::typeindex::type_id_runtime(value).pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "Could not find type: "s + e.what();
		}catch(...){
			return "Could not find type";
		}
	}


	template < typename T >
	std::string type_name(){
		try{
			return boost::typeindex::type_id< T >().pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "Could not find type: "s + e.what();
		}catch(...){
			return "Could not find type";
		}
	}


	template < typename T >
	std::string type_name_with_cvr(){
		try{
			return boost::typeindex::type_id_with_cvr< T >().pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "Could not find type: "s + e.what();
		}catch(...){
			return "Could not find type";
		}
	}


}


#endif
