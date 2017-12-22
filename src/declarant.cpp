//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/core/declarant.hpp>
#include <disposer/core/directory.hpp>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>


namespace disposer{


	void declarant::operator()(
		std::string const& type_name,
		component_maker_fn&& fn
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add component type name '" << type_name << "'";
		}, [&]{
			auto iter = directory_.component_maker_list_.insert(
				std::make_pair(type_name, std::move(fn)));

			if(!iter.second){
				throw std::logic_error(
					"component type name '" + type_name
					+ "' has been added more than one time!"
				);
			}
		});
	}

	void declarant::operator()(
		std::string const& type_name,
		module_maker_fn&& fn
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add module type name '" << type_name << "'";
		}, [&]{
			auto iter = directory_.module_maker_list_.insert(
				std::make_pair(type_name, std::move(fn)));

			if(!iter.second){
				throw std::logic_error(
					"module type name '" + type_name
					+ "' has been added more than one time!"
				);
			}
		});
	}

	void declarant::module_help(
		std::string const& type_name,
		std::string&& text
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add help for component type name '" << type_name << "'";
		}, [&]{
			auto iter = directory_.component_help_list_.insert(
				std::make_pair(type_name, std::move(text)));

			if(!iter.second){
				throw std::logic_error(
					"component type name '" + type_name
					+ "' help text has been added more than one time!"
				);
			}
		});
	}

	void declarant::component_help(
		std::string const& type_name,
		std::string&& text
	){
		logsys::log([&type_name](logsys::stdlogb& os){
			os << "add help for module type name '" << type_name << "'";
		}, [&]{
			auto iter = directory_.module_help_list_.insert(
				std::make_pair(type_name, std::move(text)));

			if(!iter.second){
				throw std::logic_error(
					"module type name '" + type_name
					+ "' help text has been added more than one time!"
				);
			}
		});
	}


}
