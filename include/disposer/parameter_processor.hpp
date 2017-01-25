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


		/// \brief Check if T is a boost::optional
		template < typename T >
		struct is_optional: std::false_type{};

		/// \brief Check if T is a boost::optional
		template < typename T >
		struct is_optional< boost::optional< T > >: std::true_type{};

		/// \brief true, if T is a boost::optional, false otherwise
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
		///
		/// Mark name as used.
		template < typename T >
		T get(std::string const& name){
			auto iter = find(name);
			if(iter == parameters_.cend()){
				throw std::runtime_error("parameter '" + name + "' not found");
			}
			return cast< T >(name, iter->second);
		}

		/// \brief Set target to value of the named parameter
		///
		/// \copydetails parameter_processor::get()
		template < typename T >
		void set(T& target, std::string const& name){
			target = get< T >(name);
		}

		/// \brief Get parameter value as type T by name, or default value if
		///        it don't exist
		///
		/// Mark name as used.
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

		/// \brief Set target to value of the named parameter, of to default
		///        value if it don't exist
		///
		/// Mark name as used.
		template < typename T, typename V >
		void set(T& target, std::string const& name, V&& default_value){
			target = get< T >(name, std::forward< V >(default_value));
		}

		/// \brief Get parameter value as type optional< T > by name
		///
		/// Optional is empty if parameter don't exist.
		///
		/// Mark name as used.
		template < typename T >
		boost::optional< T > get_optional(std::string const& name){
			auto iter = find(name);
			if(iter == parameters_.cend()) return boost::none;
			return cast< T >(name, iter->second);
		}

		/// \brief Set target to value of the named parameter
		///
		/// Target is empty if parameter don't exist.
		///
		/// Mark name as used.
		template < typename T >
		void set(boost::optional< T >& target, std::string const& name){
			target = get_optional< T >(name);
		}

		/// \brief List all unused parameters
		parameter_list unused()const{
			parameter_list result(parameters_);
			for(auto const& name: used_parameters_){
				result.erase(name);
			}
			return result;
		}

	private:
		/// \brief Get iterator to element name and mark it as used
		parameter_list::const_iterator find(std::string const& name){
			used_parameters_.insert(name);
			return parameters_.find(name);
		}

		/// \brief Convert value to type T, add error info if necessary
		///
		/// \copydetails parameter_processor::do_cast
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
		/// \throw std::logic_error if value does not fit in range of T
		template < typename T >
		static T do_cast(std::string const& value){
			if constexpr(std::is_same_v< T, bool >){
				if(value == "true") return true;
				if(value == "false") return false;
				throw std::logic_error("Can not convert to bool");
			}else if constexpr(
				std::is_same_v< T, signed char > ||
				std::is_same_v< T, unsigned char >
			){
				auto result = boost::lexical_cast< int >(value);
				if(
					result < std::numeric_limits< T >::min() ||
					result > std::numeric_limits< T >::max()
				) std::logic_error("value is not in range");
				return static_cast< T >(result);
			}else if constexpr(std::is_same_v< T, char >){
				return static_cast< char >(
					parameter_processor::do_cast< std::conditional_t<
						std::is_signed< char >::value,
						signed char, unsigned char >
					>(value)
				);
			}else{
				return boost::lexical_cast< T >(value);
			}
		}


		/// \brief Map of parameters (name & value)
		parameter_list const parameters_;

		/// \brief Set of all used parameters
		std::set< std::string > used_parameters_;
	};

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
