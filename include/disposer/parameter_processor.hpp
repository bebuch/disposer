//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer_parameter_processor_hpp_INCLUDED_
#define _disposer_parameter_processor_hpp_INCLUDED_

#include <stdexcept>
#include <limits>
#include <string>
#include <map>
#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/type_index.hpp>
#include <boost/optional.hpp>


namespace disposer{


	using parameter_list = std::map< std::string, std::string >;

	class parameter_processor{
	public:
		parameter_processor(parameter_list const& parameters): parameters_(parameters) {}

		template < typename T >
		T get(std::string const& name){
			auto iter = find(name);
			if(iter == parameters_.cend()) throw std::runtime_error("parameter '" + name + "' not found");
			return cast< T >(name, iter->second);
		}

		template < typename T >
		void set(T& target, std::string const& name){
			target = get< T >(name);
		}

		template < typename T >
		T get(std::string const& name, T&& default_value){
			// TODO: somthing like:
			// static_assert(std::is_same< T, boost::optional< typename T::value_type > >::value, "parameter can not be optional and default at the same time");

			auto iter = find(name);
			if(iter == parameters_.cend()) return std::forward< T >(default_value);
			return cast< T >(name, iter->second);
		}

		template < typename T, typename V >
		void set(T& target, std::string const& name, V&& default_value){
			target = get< T >(name, std::forward< V >(default_value));
		}

		template < typename T >
		boost::optional< T > get_optional(std::string const& name){
			auto iter = find(name);
			if(iter == parameters_.cend()) return boost::none;
			return cast< T >(name, iter->second);
		}

		template < typename T >
		void set(boost::optional< T >& target, std::string const& name){
			target = get_optional< T >(name);
		}

		parameter_list unused()const{
			parameter_list result(parameters_);
			for(auto const& name: used_parameters_){
				result.erase(name);
			}
			return result;
		}

	private:
		parameter_list::const_iterator find(std::string const& name){
			used_parameters_.insert(name);
			return parameters_.find(name);
		}

		template < typename T >
		T cast(std::string const& name, std::string const& value)try{
			return do_cast< T >(value);
		}catch(...){
			throw std::runtime_error("parameter '" + name + "' (value is '" + value + "') can not be converted to '" + boost::typeindex::type_id< T >().pretty_name() + "'");
		}

		template < typename T >
		T do_cast(std::string const& value){
			return boost::lexical_cast< T >(value);
		}

		parameter_list const parameters_;
		std::set< std::string > used_parameters_;
	};

	template <>
	inline bool parameter_processor::do_cast(std::string const& value){
		if(value == "true") return true;
		if(value == "false") return false;
		throw std::logic_error("Can not convert to bool");
	}

	template <>
	inline signed char parameter_processor::do_cast(std::string const& value){
		auto result = boost::lexical_cast< int >(value);
		if(
			result < std::numeric_limits< signed char >::min() ||
			result > std::numeric_limits< signed char >::max()
		) std::logic_error("value is not in range");
		return static_cast< signed char >(result);
	}

	template <>
	inline unsigned char parameter_processor::do_cast(std::string const& value){
		auto result = boost::lexical_cast< unsigned >(value);
		if(result > std::numeric_limits< unsigned char >::max()) std::logic_error("value is not in range");
		return static_cast< signed char >(result);
	}

	template <>
	inline char parameter_processor::do_cast(std::string const& value){
		return static_cast< char >(parameter_processor::do_cast< std::conditional_t< std::is_signed< char >::value, signed char, unsigned char > >(value));
	}

	inline std::string make_list_string(std::set< std::string > const& list){
		std::string result;
		if(list.empty()) return result;

		auto iter = list.begin();
		result += "'" + *iter;

		while(++iter != list.end()) result += "', '" + *iter;
		return result + "'";
	}


}


#endif
