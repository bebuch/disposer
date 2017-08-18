//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__config_fn__hpp_INCLUDED_
#define _disposer__core__config_fn__hpp_INCLUDED_

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/count_if.hpp>


namespace disposer{


	template < typename LogStream, typename T >
	void print_if_supported(LogStream& os, T const& v){
		auto const is_printable = hana::is_valid([](auto& x)
			->decltype((void)(std::declval< std::ostream& >() << x)){})(v);
		if constexpr(is_printable){
			os << v;
		}else{
			os << "value output on std::ostream& is not supported by type";
		}
	}



	struct parser_fn_tag;

	template < typename Fn >
	class parser_fn{
	public:
		using hana_tag = parser_fn_tag;

		constexpr parser_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit parser_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit parser_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(
				std::is_invocable_r_v< T, Fn const, Accessory const&,
					std::string_view, hana::basic_type< T > >,
				"Wrong function signature, expected: "
				"T f(auto const& iop, std::string_view value, "
				"hana::basic_type< T > type)"
			);

			return std::is_nothrow_invocable_v< Fn const, Accessory const&,
				std::string_view, hana::basic_type< T > >;
		}

		template < typename Accessory, typename T >
		T operator()(
			Accessory const& accessory,
			std::string_view value,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< Accessory, T >()){
			return accessory.log(
				[](logsys::stdlogb& os, T const* value){
					os << "parsed value";
					if(value){
						os << ": ";
						print_if_supported(os, *value);
					}
					os << " ["
						<< type_index::type_id< T >().pretty_name() << "]";
				}, [&]()noexcept(calc_noexcept< Accessory, T >())->T{
					return std::invoke(fn_, accessory, value, type);
				});
		}

	private:
		Fn fn_;
	};

	struct stream_parser_t{
		static void verify_istream(std::string_view value, std::istream& is){
			if(!is){
				throw std::runtime_error("parsing of '" + std::string(value)
					+ "' failed");
			}
			if(!is.eof()){
				std::ostringstream os;
				for(char c = is.get(); is; c = is.get()) os << c;
				throw std::runtime_error("parsing of '" + std::string(value)
					+ "' not exhaustive, rest: '"
					+ os.str() + "'");
			}
		}

		template < typename Accessory, typename T >
		T operator()(
			Accessory const& /*accessory*/,
			std::string_view value,
			hana::basic_type< T > type
		)const{
			if constexpr(type == hana::type_c< std::string >){
				return std::string(value);
			}else{
				std::istringstream is((std::string(value)));
				if constexpr(
					std::is_same_v< T, char > ||
					std::is_same_v< T, signed char > ||
					std::is_same_v< T, unsigned char >
				){
					int result;
					is >> result;
					verify_istream(value, is);
					return static_cast< T >(result);
				}else{
					if constexpr(std::is_same_v< T, bool >){
						is >> std::boolalpha;
					}

					T result;
					is >> result;
					verify_istream(value, is);
					return result;
				}
			}
		}

		template < typename Accessory, typename T >
		std::optional< T > operator()(
			Accessory const& accessory,
			std::string_view value,
			hana::basic_type< std::optional< T > >
		)const{
			return (*this)(accessory, value, hana::type_c< T >);
		}
	};

	constexpr auto stream_parser = parser_fn< stream_parser_t >{};



	struct default_value_fn_tag;

	template < typename Fn >
	class default_value_fn{
	public:
		using hana_tag = default_value_fn_tag;

		constexpr default_value_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit default_value_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit default_value_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T >
		static constexpr bool is_invocable_v()noexcept{
			return std::is_invocable_v< Fn const, Accessory const&,
				hana::basic_type< T > >;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T >
		static constexpr bool is_void_r_v()noexcept{
			if constexpr(is_invocable_v< Accessory, T >()){
				return std::is_void_v< std::invoke_result_t< Fn const,
					Accessory const&, hana::basic_type< T > > >;
			}else{
				return false;
			}
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename Accessory, typename T >
		static constexpr auto is_void_r(
			Accessory const&, hana::basic_type< T >
		)noexcept{
			if constexpr(is_void_r_v< Accessory, T >()){
				return hana::true_c;
			}else{
				return hana::false_c;
			}
		}

		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(std::is_invocable_r_v< std::optional< T >, Fn const,
				Accessory const&, hana::basic_type< T > >
				|| is_void_r_v< Accessory, T >(),
				"Wrong function signature, expected: "
				"std::optional< T > "
				"f(auto const& iop, hana::basic_type< T > type) or"
				"void f(auto const& iop, hana::basic_type< T > type)"
			);

			return std::is_nothrow_invocable_v< Fn const,
				Accessory const&, hana::basic_type< T > >;
		}

		/// \brief Operator for outputs
		template < typename Accessory, typename T >
		std::optional< T > operator()(
			Accessory const& accessory,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< Accessory, T >()){
			return accessory.log([](logsys::stdlogb& os, T const* value){
					if(value){
						os << "generated default value: ";
						print_if_supported(os, *value);
					}else{
						os << "no default value generated";
					}
					os << " [" << type_index::type_id< T >().pretty_name()
						<< "]";
				}, [&]()noexcept(calc_noexcept< Accessory, T >())->T{
					return std::invoke(fn_, accessory, type);
				});
		}


	private:
		Fn fn_;
	};


	struct auto_default_t{
		template < typename Accessory, typename T >
		void operator()(
			Accessory const&, hana::basic_type< T >
		)const noexcept{}

		template < typename Accessory, typename T >
		std::optional< T > operator()(
			Accessory const&, hana::basic_type< std::optional< T > >
		)const noexcept{ return {}; }
	};

	constexpr auto auto_default = default_value_fn(auto_default_t{});

	template < typename T >
	auto default_value(T&& value)
	noexcept(std::is_nothrow_move_constructible_v< T >){
		return default_value_fn([value = std::move(value)](auto const&, auto)
			noexcept(std::is_nothrow_copy_constructible_v< T >)
			{ return value; });
	}



	template < typename Tuple, typename Predicate, typename Default >
	constexpr auto get_or_default(
		Tuple&& tuple,
		Predicate&& predicate,
		Default&& default_value
	){
		auto const count = hana::count_if(tuple, predicate);
		static_assert(count <= hana::size_c< 1 >,
			"more than one argument with this tag");

		auto result = hana::find_if(static_cast< Tuple&& >(tuple),
			static_cast< Predicate&& >(predicate));
		if constexpr(auto const found = result != hana::nothing; found){
			return *static_cast< decltype(result)&& >(result);
		}else{
			return static_cast< Default&& >(default_value);
		}
	}


}


#endif
