//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__module_base__hpp_INCLUDED_
#define _disposer__module_base__hpp_INCLUDED_

#include "disposer.hpp"
#include "make_data.hpp"
#include "output_base.hpp"
#include "input_base.hpp"
#include "log.hpp"

#include <functional>


namespace disposer{


	struct module_not_as_start: std::logic_error{
		module_not_as_start(make_data const& data):
			std::logic_error("module type '" + data.type_name + "' can not be used as start of chain '" + data.chain + "'"){}
	};


	class module_base{
	public:
		module_base(make_data const& data, std::vector< std::reference_wrapper< input_base > >&& inputs, std::vector< std::reference_wrapper< output_base > >&& outputs = {});
		module_base(make_data const& data, std::vector< std::reference_wrapper< output_base > >&& outputs, std::vector< std::reference_wrapper< input_base > >&& inputs = {});

		module_base(module_base const&) = delete;
		module_base(module_base&&) = delete;

		module_base& operator=(module_base const&) = delete;
		module_base& operator=(module_base&&) = delete;

		virtual ~module_base() = default;


		template < typename Log >
		void log(Log&& f)const{
			::disposer::log(module_log(f));
		}

		template < typename Log, typename Body >
		decltype(auto) log(Log&& f, Body&& body)const{
			return ::disposer::log(module_log(f), static_cast< Body&& >(body));
		}


		std::string const type_name;
		std::string const chain;
		std::string const name;
		std::size_t const number;

		std::size_t const id_increase;
		std::size_t const& id;


	protected:
		virtual void trigger() = 0;


		virtual void input_ready(){}


	private:
		std::size_t id_;

		void set_id(std::size_t id);

		std::vector< std::reference_wrapper< input_base > > inputs_;
		std::vector< std::reference_wrapper< output_base > > outputs_;


		void cleanup(std::size_t id)noexcept;


		template < typename Log >
		auto module_log(Log& log)const{
			using log_t = impl::log::extract_log_t< Log >;
			return [&](log_t& os){
				os << "id(" << id << '.' << number << ") ";
				log(os);
			};
		}


	friend class chain;
	friend class ::disposer::disposer::impl;
	};


}


#endif
