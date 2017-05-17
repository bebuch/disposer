//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__output_info__hpp_INCLUDED_
#define _disposer__output_info__hpp_INCLUDED_

#include "type_index.hpp"

#include <boost/hana/type.hpp>

#include <map>
#include <sstream>
#include <stdexcept>


namespace disposer{


	namespace hana = boost::hana;


	class output_info{
	public:
		output_info(std::map< type_index, bool > const& enabled_types):
			enabled_types_(enabled_types) {}

		/// \brief true if type is enabled, false otherwise
		///
		/// \throw std::logic_error if type doesn't exist
		bool is_type_enabled(type_index const& type)const{
			auto iter = enabled_types_.find(type);
			if(iter == enabled_types_.end()){
				std::ostringstream os;
				os << "requested type [" << type.pretty_name()
					<< "] is not in the connected output type list, valid "
					"types are";

				bool first = true;
				for(auto const& [type, active]: enabled_types_){
					using namespace std::literals::string_view_literals;
					if(!first){ os << ", "; }else{ first = false; }
					os << '[' << type.pretty_name() << "]("
						<< (active ? "on"sv : "off"sv) << ')';
				}

				throw std::logic_error(os.str());
			}

			return iter->second;
		}


		/// \brief true if type is enabled, false otherwise
		///
		/// \throw std::logic_error if type doesn't exist
		template < typename T >
		bool is_type_enabled(hana::basic_type< T > const&)const{
			return is_type_enabled(type_index::type_id<
				typename hana::basic_type< T >::type >());
		}


	private:
		std::map< type_index, bool > const enabled_types_;
	};


}


#endif
