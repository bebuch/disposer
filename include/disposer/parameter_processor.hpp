//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__parameter_processor__hpp_INCLUDED_
#define _disposer__parameter_processor__hpp_INCLUDED_

#include <stdexcept>
#include <limits>
#include <string>
#include <map>
#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/type_index.hpp>
#include <boost/optional.hpp>


namespace disposer{


	namespace detail{


		template < typename T >
		struct is_optional: std::false_type{};

		template < typename T >
		struct is_optional< boost::optional< T > >: std::true_type{};

		template < typename T >
		constexpr bool is_optional_v = is_optional< T >::value;


	}


	/// \brief A parameter has a name and a value
	using parameter_list = std::map< std::string, std::string >;

	/// \brief Access parameter value by name and convert to C++ type
	class parameter_processor{
	public:
		/// \brief Init with a list of parameters
		parameter_processor(parameter_list const& parameters):
			parameters_(parameters) {}

		/// \brief Get value as type T by name
		///
		/// \throw std::runtime_error if parameter don't exist
		template < typename T >
		T get(std::string const& name){
			auto iter = find(name);
			if(iter == parameters_.cend()){
				throw std::runtime_error("parameter '" + name + "' not found");
			}
			return cast< T >(name, iter->second);
		}

		/// \brief Convert value to type of target and assing it
		///
		/// \copydetails parameter_processor::get()
		template < typename T >
		void set(T& target, std::string const& name){
			target = get< T >(name);
		}

		template < typename T >
		T get(std::string const& name, T&& default_value){
			using type = std::remove_cv_t< std::remove_reference_t< T > >;
			static_assert(
				!detail::is_optional_v< type >,
				"parameter can not be optional and default at the same time"
			);

			auto iter = find(name);
			if(iter == parameters_.cend()){
				return std::forward< T >(default_value);
			}
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

		/// \brief Convert value to type T, add error info if necessary
		///
		/// \copydetails parameter_processor::do_cast(value)
		template < typename T >
		T cast(std::string const& name, std::string const& value)try{
			return do_cast< T >(value);
		}catch(...){
			throw std::runtime_error(
				"parameter '" + name + "' (value is '" + value +
				"') can not be converted to '" +
				boost::typeindex::type_id< T >().pretty_name() + "'"
			);
		}

		/// \brief Convert value to type T
		///
		/// \throw boost::bad_lexical_cast if value is not convertible to T
		template < typename T >
		static T do_cast(std::string const& value){
			return boost::lexical_cast< T >(value);
		}

		parameter_list const parameters_;
		std::set< std::string > used_parameters_;
	};

	/// \copydoc parameter_processor::do_cast(value)
	template <>
	inline bool parameter_processor::do_cast(std::string const& value){
		if(value == "true") return true;
		if(value == "false") return false;
		throw std::logic_error("Can not convert to bool");
	}

	/// \copydoc parameter_processor::do_cast(value)
	template <>
	inline signed char parameter_processor::do_cast(std::string const& value){
		auto result = boost::lexical_cast< int >(value);
		if(
			result < std::numeric_limits< signed char >::min() ||
			result > std::numeric_limits< signed char >::max()
		) std::logic_error("value is not in range");
		return static_cast< signed char >(result);
	}

	/// \copydoc parameter_processor::do_cast(value)
	template <>
	inline unsigned char parameter_processor::do_cast(
		std::string const& value
	){
		auto result = boost::lexical_cast< unsigned >(value);
		if(result > std::numeric_limits< unsigned char >::max()){
			std::logic_error("value is not in range");
		}
		return static_cast< signed char >(result);
	}

	/// \copydoc parameter_processor::do_cast(value)
	template <>
	inline char parameter_processor::do_cast(std::string const& value){
		return static_cast< char >(
			parameter_processor::do_cast< std::conditional_t<
				std::is_signed< char >::value, signed char, unsigned char >
			>(value)
		);
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
