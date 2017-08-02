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

#include "../config/embedded_config.hpp"

#include "output_base.hpp"

#include <boost/hana/type.hpp>

#include <map>
#include <sstream>
#include <stdexcept>


namespace disposer{


	namespace hana = boost::hana;


	struct input_key;

	/// \brief Provides information which output types are enabled
	class output_info{
	public:
		/// \brief Constructor
		output_info(output_base* output, bool last_use)
			: output_(output)
			, last_use_(last_use)
			, enabled_types_(output->enabled_types()) {}


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


		/// \brief pointer to an object in a former module
		output_base* output()const noexcept{
			return output_;
		}

		/// \brief true if this is the last use of the config file variable
		bool last_use()const noexcept{
			return last_use_;
		}


		/// \brief Throw if connected output has an enabled type which is not
		///        a type of the input
		template < typename ... T >
		void verify_nothing_enabled_except(
			input_key const&,
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
					os << "[" << type.pretty_name() << "]";
				}
				os << "; legal types are: ";
				first = true;
				for(auto const& type: legal_types){
					if(!first){ os << ", "; }else{ first = false; }
					os << "[" << type.pretty_name() << "]";
				}

				throw std::runtime_error(os.str());
			}
		}


	private:
		output_base* output_;
		bool last_use_;
		std::map< type_index, bool > const enabled_types_;
	};

	/// \brief Create output_info if an output is connected
	std::optional< output_info > make_output_info(
		input_list const& list,
		std::string const& input_name
	){
		auto iter = list.find(input_name);

		// output referes to an output in a former module
		return iter != list.end()
			? std::optional< output_info >(std::in_place,
				std::get< 0 >(iter->second), std::get< 1 >(iter->second))
			: std::optional< output_info >();

	}


}


#endif
