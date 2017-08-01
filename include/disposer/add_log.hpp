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


	template < typename LogF >
	constexpr bool is_simple_log_fn =
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		std::is_callable_v< LogF(logsys::stdlogb&) >;
#else
		std::is_invocable_v< LogF, logsys::stdlogb& >;
#endif

	template < typename LogF, typename T >
	constexpr bool is_extended_log_fn =
// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
		std::is_callable_v< LogF(logsys::stdlogb&, T) >;
#else
		std::is_invocable_v< LogF, logsys::stdlogb&, T >;
#endif


	template < typename Derived >
	class add_log{
	public:
		/// \brief Add a line to the log
		template < typename LogF >
		void log(LogF&& f)const{
			static_assert(is_simple_log_fn< LogF >,
				"expected a log call of the form: "
				"'.log([](logsys::stdlogb&){})'");

			logsys::log< logsys::stdlogb >(simple_impl(f));
		}

		/// \brief Add a line to the log with linked code block
		template < typename LogF, typename Body >
		decltype(auto) log(LogF&& f, Body&& body)const{
			using result_type = logsys::detail::result_as_ptr_t< Body >;

			if constexpr(std::is_void_v< result_type >){
				static_assert(is_simple_log_fn< LogF >,
					"expected a log call of the form: "
					"'.log([](logsys::stdlogb&){}, []{})'");

				logsys::log< logsys::stdlogb >(simple_impl(f), body);
			}else{
				static_assert(is_simple_log_fn< LogF >
					|| is_extended_log_fn< LogF, result_type >,
					"expected a log call of the form: "
					"'.log([](logsys::stdlogb&){}, []{ return ...; })' or "
					"'.log([](logsys::stdlogb&, auto const* result){}, "
					"[]{ return ...; })'");

				if constexpr(is_simple_log_fn< LogF >){
					return logsys::log< logsys::stdlogb >(simple_impl(f), body);
				}else{
					return logsys::log< logsys::stdlogb >(
						extended_impl< result_type >(f), body);
				}
			}
		}

		/// \brief Add a line to the log with linked code block and catch all
		///        exceptions
		template < typename LogF, typename Body >
		decltype(auto) exception_catching_log(LogF&& f, Body&& body)const{
			using result_type = logsys::detail::result_as_ptr_t< Body >;

			if constexpr(std::is_void_v< result_type >){
				static_assert(is_simple_log_fn< LogF >,
					"expected a log call of the form: "
					"'.exception_catching_log([](logsys::stdlogb&){}, []{})'");

				logsys::exception_catching_log< logsys::stdlogb >(
					simple_impl(f), body);
			}else{
				static_assert(is_simple_log_fn< LogF >
					|| is_extended_log_fn< LogF, result_type >,
					"expected a log call of the form: "
					"'.exception_catching_log([](logsys::stdlogb&){}, "
					"[]{ return ...; })' or "
					"'.exception_catching_log([](logsys::stdlogb&, "
					"auto const* result){}, "
					"[]{ return ...; })'");

				if constexpr(is_simple_log_fn< LogF >){
					return logsys::exception_catching_log< logsys::stdlogb >(
						simple_impl(f), body);
				}else{
					return logsys::exception_catching_log< logsys::stdlogb >(
						extended_impl< result_type >(f), body);
				}
			}
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
		template < typename Log >
		void add_prefix(Log& os)const{
			static_cast< Derived const& >(*this).log_prefix(log_key(), os);
		}

		/// \brief Helper for log message functions
		template < typename LogF >
		auto simple_impl(LogF& log)const{
			return [&](logsys::stdlogb& os){
				add_prefix(os);
				log(os);
			};
		}

		/// \brief Helper for log message functions
		template < typename T, typename LogF >
		auto extended_impl(LogF& log)const{
			return [&](logsys::stdlogb& os, T result){
				add_prefix(os);
				log(os, result);
			};
		}
	};


}


#endif
