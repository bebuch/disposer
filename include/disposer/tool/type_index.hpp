//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__type_index__hpp_INCLUDED_
#define _disposer__tool__type_index__hpp_INCLUDED_

#include <boost/functional/hash.hpp>
#include <boost/type_index/ctti_type_index.hpp>

#include <string>


namespace std{

	// make ctti_type_index hashable for unordered_map and unordered_set
	template <> struct hash< boost::typeindex::ctti_type_index >:
		boost::hash< boost::typeindex::ctti_type_index >{};

}


namespace disposer{


	using type_index = boost::typeindex::ctti_type_index;



	template < typename T >
	std::string type_name(T&& value){
		try{
			return type_index::type_id_runtime(value).pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "could not find type: "s + e.what();
		}catch(...){
			return "could not find type";
		}
	}


	template < typename T >
	std::string type_name(){
		try{
			return type_index::type_id< T >().pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "could not find type: "s + e.what();
		}catch(...){
			return "could not find type";
		}
	}


	template < typename T >
	std::string type_name_with_cvr(){
		try{
			return type_index::type_id_with_cvr< T >().pretty_name();
		}catch(std::exception const& e){
			using namespace std::literals::string_literals;
			return "could not find type: "s + e.what();
		}catch(...){
			return "could not find type";
		}
	}


}


#endif
