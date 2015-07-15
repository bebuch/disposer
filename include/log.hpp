//-----------------------------------------------------------------------------
// Copyright (c) 2015 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _tools_log_hpp_INCLUDED_
#define _tools_log_hpp_INCLUDED_

#include <boost/hana.hpp>
#include <boost/hana/ext/std/utility.hpp>
#include <boost/hana/ext/std/type_traits.hpp>

#include <boost/optional.hpp>


namespace disposer{


	namespace impl{ namespace log{


		template < typename Function >
		struct extract_log_from_function{};

		template < typename F, typename R, typename Log >
		struct extract_log_from_function< R(F::*)(Log&) >{
			using type = Log;
		};

		template < typename F, typename R, typename Log >
		struct extract_log_from_function< R(F::*)(Log&)const >{
			using type = Log;
		};

		template < typename F, typename R, typename Log >
		struct extract_log_from_function< R(F::*)(Log&)volatile >{
			using type = Log;
		};

		template < typename F, typename R, typename Log >
		struct extract_log_from_function< R(F::*)(Log&)volatile const >{
			using type = Log;
		};

		template < typename Function >
		struct extract_log{
			using type = typename extract_log_from_function< decltype(&Function::operator()) >::type;
		};

		template < typename Function >
		struct extract_log< Function& >{
			using type = typename extract_log< Function >::type;
		};

		template < typename Log, typename R >
		struct extract_log< R(Log&) >{
			using type = Log;
		};

		template < typename Log, typename R >
		struct extract_log< R(*)(Log&) >{
			using type = Log;
		};

		template < typename Function >
		using extract_log_t = typename extract_log< Function >::type;


		template < typename F, typename Log >
		inline void exec_log(F& f, Log& log){
			auto has_pre = boost::hana::is_valid([](auto&& x)->decltype((void)x.pre()){});
			auto has_post = boost::hana::is_valid([](auto&& x)->decltype((void)x.post()){});

			boost::hana::if_(has_pre(log),
				[](auto& log){ log.pre(); },
				[](auto&){}
			)(log);

			f(log);

			boost::hana::if_(has_post(log),
				[](auto& log){ log.post(); },
				[](auto&){}
			)(log);

			log.exec();
		}

		template < typename F, typename Body, typename Log >
		inline decltype(auto) exec_body(F& f, Body& body, Log& log)try{
			return body();
		}catch(...){
			auto has_failed = boost::hana::is_valid([](auto&& x)->decltype((void)x.failed()){});

			boost::hana::if_(has_failed(log),
				[](auto& log){ log.failed(); },
				[](auto&){}
			)(log);

			exec_log(f, log);

			throw;
		}

		template < typename F, typename Body, typename Log >
		inline auto exec_exception_catching_body(F& f, Body& body, Log& log){
			constexpr auto is_void = boost::hana::traits::is_void(boost::hana::type< decltype(body()) >);
			try{
				return boost::hana::if_(is_void,
					[](auto&& body){ body(); return true; },
					[](auto&& body){ return boost::optional< decltype(body()) >(body()); }
				)(static_cast< Body&& >(body));
			}catch(std::exception const& error){
				log.set_exception(error);
			}catch(...){
				log.unknown_exception();
			}

			return boost::hana::if_(is_void,
				[](auto&&){ return false; },
				[](auto&& body){ return boost::optional< decltype(body()) >(); }
			)(static_cast< Body&& >(body));

		}

		auto has_exec = boost::hana::is_valid([](auto&& x)->decltype((void)x.exec()){});
		auto has_have_body = boost::hana::is_valid([](auto&& x)->decltype((void)x.have_body()){});


	} }


	template < typename Log >
	inline void log(Log&& f){
		using log_t = impl::log::extract_log_t< Log >;

		log_t log;

		static_assert(impl::log::has_exec(log), "In 'log([](Log& os){ ... })' have 'os.exec()' to be a callable expression.");

		impl::log::exec_log(f, log);
	}

	template < typename Log, typename Body >
	inline decltype(auto) log(Log&& f, Body&& body){
		using log_t = impl::log::extract_log_t< Log >;

		log_t log;

		static_assert(impl::log::has_exec(log), "In 'log([](Log& os){ ... }, []{ ... })' have 'os.exec()' to be a callable expression.");

		boost::hana::if_(impl::log::has_have_body(log),
			[](auto& log){ log.have_body(); },
			[](auto&){}
		)(log);

		return boost::hana::if_(boost::hana::traits::is_void(boost::hana::type< decltype(body()) >),
			[](auto&& f, auto&& body, auto& log){
				impl::log::exec_body(f, body, log);
				impl::log::exec_log(f, log);
			},
			[](auto&& f, auto&& body, auto& log)->decltype(auto){
				decltype(auto) result = impl::log::exec_body(f, body, log);
				impl::log::exec_log(f, log);
				return result;
			}
		)(static_cast< Log&& >(f), static_cast< Body&& >(body), log);
	}

	/// \brief Catch all exceptions
	/// Call the function and catch all exceptions throwing by the function. The name is emited
	/// via error_log together with the exception message.
	///
	/// As function the usage of a Lambda function is possible, which captures all variables by reference. ([&]{/* ... */})
	///
	/// If the Lambda function does not return anything, result will be a bool, indicating with false whether
	/// an exception appeared. Otherwise, the result will be a type that is convertible to bool. If and only
	/// if the conversion becomes true, accessability to the function result using member-function result()
	/// is permitted. Otherwise, result() will throw a std::logic_error.
	template < typename Log, typename Body >
	inline auto exception_catching_log(Log&& f, Body&& body){
		using log_t = impl::log::extract_log_t< Log >;

		log_t log;

		auto has_set_exception = boost::hana::is_valid([](auto&& x)->decltype((void)x.set_exception(std::declval< std::exception >())){});
		auto has_unknown_exception = boost::hana::is_valid([](auto&& x)->decltype((void)x.unknown_exception()){});

		static_assert(impl::log::has_exec(log), "In 'exception_catching_log([](Log& os){ ... }, []{ ... })' have 'os.exec()' to be a callable expression.");
		static_assert(has_set_exception(log), "In 'exception_catching_log([](Log& os){ ... }, []{ ... })' have 'os.set_exception(std::declval< std::exception >())' to be a callable expression.");
		static_assert(has_unknown_exception(log), "In 'exception_catching_log([](Log& os){ ... }, []{ ... })' have 'os.unknown_exception()' to be a callable expression.");

		boost::hana::if_(impl::log::has_have_body(log),
			[](auto& log){ log.have_body(); },
			[](auto&){}
		)(log);

		auto result = impl::log::exec_exception_catching_body(f, body, log);
		impl::log::exec_log(f, log);
		return result;
	}



}


#endif
