//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__add_log__hpp_INCLUDED_
#define _disposer__add_log__hpp_INCLUDED_

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>

#include <boost/hana/type.hpp>


namespace disposer{


	namespace hana = boost::hana;


	template < typename Derived >
	class add_log;


	/// \brief Class disposer access key
	struct log_key{
	private:
		/// \brief Constructor
		constexpr log_key()noexcept = default;

		template < typename Derived >
		friend class add_log;
	};


	template < typename Derived >
	class add_log{
	public:
		/// \brief Add a line to the log
		template < typename Log >
		void log(Log&& f)const{
			logsys::log(impl(f));
		}

		/// \brief Add a line to the log with linked code block
		template < typename Log, typename Body >
		decltype(auto) log(Log&& f, Body&& body)const{
			return logsys::log(module_log(f), static_cast< Body&& >(body));
		}

		/// \brief Add a line to the log with linked code block and catch all
		///        exceptions
		template < typename Log, typename Body >
		decltype(auto) exception_catching_log(Log&& f, Body&& body)const{
			return logsys::exception_catching_log(
				module_log(f), static_cast< Body&& >(body));
		}

	protected:
		constexpr add_log()noexcept{
			static_assert(std::is_base_of_v< add_log, Derived >);

			auto log_prefix_implemented =
				hana::is_valid([](auto derived_type)->decltype(
					std::declval< typename decltype(derived_type)::type >()
					.log_prefix(log_key(), std::declval< logsys::stdlogb& >())
				){})(hana::type_c< Derived const >);
			static_assert(log_prefix_implemented,
				"Derived must implement log_prefix function");
		}

	private:
		/// \brief Helper for log message functions
		template < typename Log >
		auto impl(Log& log)const{
			return [&](logsys::stdlogb& os){
				static_cast< Derived const& >(*this).log_prefix(log_key(), os);

// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
				static_assert(
					std::is_callable_v< Log(logsys::stdlogb&) >,
					"log type must be logsys::stdlogb"
				);
#else
				static_assert(
					std::is_invocable_v< Log, logsys::stdlogb& >,
					"log type must be logsys::stdlogb"
				);
#endif

				log(os);
			};
		}
	};


}


#endif
