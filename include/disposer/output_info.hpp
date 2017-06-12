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


	struct input_maker_key;

	/// \brief Provides information which output types are enabled
	class output_info{
	public:
		/// \brief Constructor
		output_info(std::map< type_index, bool > const& enabled_types):
			enabled_types_(enabled_types) {}


		/// \brief true if type exists and is enabled, false otherwise
		bool is_enabled(type_index const& type)const{
			auto iter = enabled_types_.find(type);
			if(iter == enabled_types_.end()) return false;
			return iter->second;
		}

		/// \brief true if type exists and is enabled, false otherwise
		template < typename T >
		bool is_enabled(hana::basic_type< T > const&)const{
			return is_enabled(type_index::type_id< T >());
		}


		/// \brief Throw if connected output has an enabled type which is not
		///        a type of the input
		template < typename ... T >
		void verify_nothing_enabled_except(
			input_maker_key const&,
			hana::basic_type< T > ...
		)const{
			std::set< type_index > const legal_types{
				type_index::type_id< T >() ...
			};

			std::set< type_index > illegal_types;
			for(auto const& [type, enabled]: enabled_types_){
				if(!enabled) continue;
				if(legal_types.find(type) != legal_types.end()) continue;
				illegal_types.insert(type);
			}

			if(!illegal_types.empty()){
				std::ostringstream os;
				os << "the connected output has types enabled, which are not "
					"in in the input type list; the illegal types are: ";
				auto first = true;
				for(auto const& type: illegal_types){
					if(!first){ os << ", "; }else{ first = false; }
					os << '[' << type.pretty_name() << ']';
				}
				os << "; legal types are: ";
				first = true;
				for(auto const& type: legal_types){
					if(!first){ os << ", "; }else{ first = false; }
					os << '[' << type.pretty_name() << ']';
				}

				throw std::runtime_error(os.str());
			}
		}


	private:
		std::map< type_index, bool > const enabled_types_;
	};


}


#endif
