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

#include "make_data.hpp"
#include "output_base.hpp"
#include "input_base.hpp"
#include "log.hpp"

#include <functional>


namespace disposer{


	namespace impl{ namespace module_base{


		template < typename Log >
		struct module_log{
			using log_t = impl::log::extract_log_t< Log >;

			void operator()(log_t& os)const{
				os << "id(" << id << '.' << number << ") "; log(os);
			}

			Log& log;
			std::size_t id;
			std::size_t number;
		};

		template < typename Log >
		inline auto make_module_log(Log& log, std::size_t id, std::size_t number){
			return module_log< Log >{log, id, number};
		}


	} }


	struct module_not_as_start: std::logic_error{
		module_not_as_start(make_data const& data):
			std::logic_error("module type '" + data.type_name + "' can not be used as start of chain '" + data.chain + "'"){}
	};


	class module_base{
	public:
		module_base(make_data const& data);

		module_base(module_base const&) = delete;
		module_base(module_base&&) = delete;

		module_base& operator=(module_base const&) = delete;
		module_base& operator=(module_base&&) = delete;

		virtual ~module_base() = default;

		virtual void trigger() = 0;

		void cleanup(std::size_t id)noexcept;

		template < typename Log >
		void log(Log&& f)const{
			disposer::log(impl::module_base::make_module_log(f, id, number));
		}

		template < typename Log, typename Body >
		decltype(auto) log(Log&& f, Body&& body)const{
			return disposer::log(impl::module_base::make_module_log(f, id, number), static_cast< Body&& >(body));
		}

		std::string const type_name;
		std::string const chain;
		std::string const name;
		std::size_t const number;

		std::size_t const& id;

	protected:
		output_list outputs;
		input_list inputs;

	private:
		std::size_t id_;

	friend class chain;
	friend class disposer;
	};


	using module_ptr = std::unique_ptr< module_base >;


}


#endif
