//-----------------------------------------------------------------------------
// Copyright (c) 2017-2018 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__default_value_fn__hpp_INCLUDED_
#define _disposer__core__default_value_fn__hpp_INCLUDED_

#include "dimension_referrer.hpp"

#include "../tool/assisted_to_string.hpp"
#include "../tool/ct_pretty_name.hpp"
#include "../tool/false_c.hpp"

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <boost/hana/type.hpp>
#include <boost/hana/traits.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/remove_if.hpp>

#include <optional>


namespace disposer{



	struct no_value{};


	template < typename Fn >
	class default_value_help_fn{
	public:
		constexpr default_value_help_fn()noexcept(
			std::is_nothrow_default_constructible_v< Fn >
		)
			: fn_() {}

		constexpr default_value_help_fn(Fn const& fn)noexcept(
			std::is_nothrow_copy_constructible_v< Fn >
		)
			: fn_(fn) {}

		constexpr default_value_help_fn(Fn&& fn)noexcept(
			std::is_nothrow_move_constructible_v< Fn >
		)
			: fn_(std::move(fn)) {}


		template <
			typename GenFn,
			template < typename ... > typename Template,
			std::size_t ... Ds,
			typename ... DTs >
		std::string operator()(
			GenFn const& fn,
			dimension_referrer< Template, Ds ... >,
			dimension_list< DTs ... >
		)const{
			if constexpr(std::is_invocable_v< Fn const >){
				static_assert(std::is_invocable_r_v< std::string, Fn const >,
					"Wrong function signature for default_value_help_fn, "
					"found:\n"
					"  H function()\n"
					"but with an H that is not convertible to std::string");
				// user defined type independent function
				return "user defined default value help: "
					+ std::string(std::invoke(fn_));
			}else{
				constexpr dimension_converter< dimension_list< DTs ... >,
					Template, Ds ... > convert;
				constexpr auto types =
					hana::transform(convert.indexes, [](auto index){
							return hana::type_c< typename
								decltype(convert.value_type_of(index))::type >;
						});
				constexpr auto valid_types =
					hana::remove_if(types, hana::traits::is_void);
				constexpr auto unique_types =
					hana::to_tuple(hana::to_set(valid_types));
				if constexpr(hana::size(unique_types) == hana::size_c< 0 >){
					return "no default value";
				}else if constexpr(
					hana::size(unique_types) == hana::size_c< 1 >
				){
					return "default value: "
						+ print_by_type(fn, unique_types[hana::size_c< 0 >]);
				}else{
					std::string help_text = "default values by type: ";
					hana::for_each(unique_types,
						[this, &fn, &help_text](auto type){
							help_text += "\n* " + ct_pretty_name<
									typename decltype(type)::type >() + ": "
								+ this->print_by_type(fn, type);
						});
					return help_text;
				}
			}
		}


	private:
		template < typename GenFn, typename T >
		std::string print_by_type(GenFn const& fn, hana::basic_type< T >)const{
			static_assert(
				std::is_invocable_v< Fn const, T const& > ||
				std::is_invocable_v< Fn const, no_value,
					hana::basic_type< T > >,
				"Wrong function signature for default_value_help_fn, expected "
				"one of:\n"
				"  H function()\n"
				"  H function(T const& default_value)\n"
				"  H function(no_value, hana::basic_type< T > "
				"default_value_type)\n"
				"where H is convertible to std::string");


			if constexpr(std::is_invocable_v< Fn const, T const& >){
				static_assert(
					std::is_invocable_r_v< std::string, Fn const, T const& >,
					"Wrong function signature for default_value_help_fn, "
					"found:\n"
					"  H function(T const& default_value)"
					"but with an H that is not convertible to std::string");

				if constexpr(std::is_invocable_v< GenFn const >){
					if constexpr(
						std::is_void_v< std::invoke_result_t< GenFn const > >
					){
						return "no default value";
					}else{
						T const default_value(std::invoke(fn));
						return std::invoke(fn_, default_value);
					}
				}else if constexpr(
					std::is_invocable_v< GenFn const, hana::basic_type< T > >
				){
					if constexpr(
						std::is_void_v< std::invoke_result_t< GenFn const,
							hana::basic_type< T > > >
					){
						return "no default value";
					}else{
						T const default_value(
							std::invoke(fn, hana::type_c< T >));
						return std::invoke(fn_, default_value);
					}
				}else{
					static_assert(detail::false_c< GenFn >,
						"default_value_fn doesn't have the signature:\n"
						"  R function()\n"
						"  R function(hana::basic_type< T > type)\n"
						"its signature is invalid or depends on Ref by "
						"signature:\n"
						"  R function(hana::basic_type< T > type, "
						"auto ref)\n"
						"if it depends on Ref, default_value_help_fn "
						"must have one of these signatures\n"
						"  H function()\n"
						"  H function(no_value, hana::basic_type< T > "
						"default_value_type)\n"
						"where H is convertible to std::string");
				}
			}else{
				static_assert(std::is_invocable_r_v< std::string, Fn const,
					no_value, hana::basic_type< T > >,
					"Wrong function signature for default_value_help_fn, "
					"found:\n"
					"  H function(no_value, hana::basic_type< T > "
					"default_value_type)\n"
					"but with an H that is not convertible to std::string");

				return std::invoke(fn_, no_value{}, hana::type_c< T >);
			}
		}

		Fn fn_;
	};


	struct default_value_help_generator{
		template < typename T >
		std::string operator()(T const& v)const{
			return assisted_to_string(v);
		}
	};


	struct default_value_fn_tag;

	template < typename Fn, typename HelpFn >
	class default_value_fn{
	public:
		using hana_tag = default_value_fn_tag;

		constexpr default_value_fn()noexcept(
			std::is_nothrow_default_constructible_v< Fn > &&
			std::is_nothrow_default_constructible_v< HelpFn >
		)
			: fn_()
			, help_fn() {}

		constexpr default_value_fn(
			HelpFn const& help_fn = default_value_help_generator{}
		)noexcept(
			std::is_nothrow_default_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_()
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			HelpFn&& help_fn
		)noexcept(
			std::is_nothrow_default_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_()
			, help_fn(std::move(help_fn)) {}

		constexpr default_value_fn(
			Fn const& fn,
			HelpFn const& help_fn = default_value_help_generator{}
		)noexcept(
			std::is_nothrow_copy_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_(fn)
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			Fn const& fn,
			HelpFn&& help_fn
		)noexcept(
			std::is_nothrow_copy_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_(fn)
			, help_fn(std::move(help_fn)) {}

		constexpr default_value_fn(
			Fn&& fn,
			HelpFn const& help_fn = default_value_help_generator{}
		)noexcept(
			std::is_nothrow_move_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_(std::move(fn))
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			Fn&& fn,
			HelpFn&& help_fn
		)noexcept(
			std::is_nothrow_move_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_(std::move(fn))
			, help_fn(std::move(help_fn)) {}


		template < typename T, typename Ref >
		static constexpr bool is_invocable_v()noexcept{
			return
				std::is_invocable_v< Fn const, hana::basic_type< T >,
					Ref > ||
				std::is_invocable_v< Fn const, hana::basic_type< T > > ||
				std::is_invocable_v< Fn const >;
		}

		template < typename T, typename Ref >
		static constexpr auto invoke_result_type()noexcept{
			static_assert(is_invocable_v< T, Ref >(),
				"Wrong function signature for default_value_fn, expected one "
				"of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto ref)\n"
				"where R is void or convertible to T");

			auto type = []{
					if constexpr(std::is_invocable_v< Fn const >){
						return hana::type_c< std::invoke_result_t< Fn const > >;
					}else if constexpr(
						std::is_invocable_v< Fn const, hana::basic_type< T > >
					){
						return hana::type_c< std::invoke_result_t< Fn const,
							hana::basic_type< T > > >;
					}else{
						return hana::type_c< std::invoke_result_t< Fn const,
							hana::basic_type< T >, Ref > >;
					}
				}();

			auto const is_result_valid =
				hana::traits::is_void(type) ||
				hana::traits::is_convertible(type, hana::type_c< T >);
			static_assert(is_result_valid,
				"Wrong function signature for default_value_fn, expected one "
				"of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto ref)\n"
				"where R must be void or convertible to T");

			return type;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename T, typename Ref >
		static constexpr auto is_void_r(
			hana::basic_type< T >, Ref
		)noexcept{
			return hana::traits::is_void(invoke_result_type< T, Ref >());
		}

		template < typename T, typename Ref >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(is_invocable_v< T, Ref >(),
				"Wrong function signature for default_value_fn, expected one "
				"of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto ref)\n"
				"where R is void or convertible to T");

			if constexpr(std::is_invocable_v< Fn const >){
				return std::is_nothrow_invocable_v< Fn const >;
			}else if constexpr(
				std::is_invocable_v< Fn const, hana::basic_type< T > >
			){
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T > >;
			}else{
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T >, Ref >;
			}
		}

		/// \brief Generate a value via the user defined function
		template < typename T, typename Ref >
		T operator()(
			std::string_view parameter_name,
			hana::basic_type< T > type,
			Ref ref
		)const noexcept(calc_noexcept< T, Ref >()){
			(void)type; // Silance GCC
			return ref.log(
				[parameter_name](
					logsys::stdlogb& os,
					std::optional< T > const& value
				){
					os << "parameter(" << parameter_name << ") ";
					if(value){
						os << "generated default value: "
							<< assisted_to_string(*value, hana::true_c);
					}else{
						os << "no default value generated";
					}
					os << " [" << ct_pretty_name< T >() << "]";
				}, [&]()noexcept(calc_noexcept< T, Ref >())->T{
					if constexpr(std::is_invocable_v< Fn const >){
						return std::invoke(fn_);
					}else if constexpr(
						std::is_invocable_v< Fn const, hana::basic_type< T > >
					){
						return std::invoke(fn_, type);
					}else{
						return std::invoke(fn_, type, ref);
					}
				});
		}


		template <
			template < typename ... > typename Template,
			std::size_t ... Ds,
			typename ... DTs >
		std::string help_text_fn(
			dimension_referrer< Template, Ds ... > ref,
			dimension_list< DTs ... > dims
		)const{
			return help_fn(fn_, ref, dims);
		}


	private:
		Fn fn_;

		/// \brief Help text generator function
		default_value_help_fn< HelpFn > const help_fn;
	};


	template < typename Fn >
	default_value_fn(Fn)
	-> default_value_fn< Fn, default_value_help_generator >;

	template < typename Fn, typename HelpFn >
	default_value_fn(Fn, HelpFn) -> default_value_fn< Fn, HelpFn >;


	struct auto_default_t{
		template < typename T >
		constexpr void operator()(hana::basic_type< T >)const noexcept{}

		template < typename T >
		constexpr std::optional< T > operator()(
			hana::basic_type< std::optional< T > >
		)const noexcept{ return {}; }
	};

	constexpr auto auto_default = default_value_fn(auto_default_t{});

	template < typename T >
	constexpr auto default_value(T&& value)
	noexcept(std::is_nothrow_move_constructible_v< T >){
		return default_value_fn(
			[value = static_cast< T&& >(value)]()
			noexcept(std::is_nothrow_copy_constructible_v< T >){
				return value;
			});
	}


	namespace detail{


		template < typename T >
		constexpr bool is_nothrow_default_constructible_v = []{
				static_assert(std::is_default_constructible_v< T >,
					"type is not default constructible, you can't use "
					"default_value(), use default_value(value) instead");

				return std::is_nothrow_default_constructible_v< T >;
			}();


	}


	constexpr auto default_value()noexcept{
		return default_value_fn([](auto const type)
			noexcept(detail::is_nothrow_default_constructible_v<
				typename decltype(type)::type >
			){
				return typename decltype(type)::type();
			});
	}



}


#endif
