//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__core__default_value_fn__hpp_INCLUDED_
#define _disposer__core__default_value_fn__hpp_INCLUDED_

#include "dimension_referrer.hpp"

#include "../tool/print_if_supported.hpp"
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
			if constexpr(std::is_invocable_r_v< std::string, Fn const >){
				// user defined type independent function
				return "user defined default value help:\n" + std::invoke(fn_);
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
					return print_by_type(fn, unique_types[hana::size_c< 0 >]);
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
				std::is_invocable_r_v< std::string, Fn const,
					hana::basic_type< T > > ||
				std::is_invocable_r_v< std::string, Fn const, T const& >,
				"Wrong function signature, expected one of:\n"
				"  H function()\n"
				"  H function(hana::basic_type< T > default_value_type)\n"
				"  H function(T const& default_value)\n"
				"where H is convertible to std::string"
			);

			if constexpr(!std::is_invocable_v< Fn const, T const& >){
				return std::invoke(fn_, hana::type_c< T >);
			}else if constexpr(std::is_invocable_v< GenFn const >){
				if constexpr(
					std::is_void_v< std::invoke_result_t< GenFn const > >
				){
					return "no default value";
				}else{
					T const default_value(std::invoke(fn));
					return "default value: "
						+ std::invoke(fn_, default_value);
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
					return "default value: "
						+ std::invoke(fn_, default_value);
				}
			}else{
				static_assert(detail::false_c< GenFn >,
					"default_value_fn doesn't have the signature:\n"
					"  R function()\n"
					"  R function(hana::basic_type< T > type)\n"
					"its signature is invalid or depends on Accessory by "
					"signature:\n"
					"  R function(hana::basic_type< T > type, auto accessory)\n"
					"default_value_help_fn must have one of these signatures\n"
					"  H function()\n"
					"  H function(hana::basic_type< T > default_value_type)\n"
					"where H is convertible to std::string"
				);
			}
		}

		Fn fn_;
	};


	struct default_value_help_generator{
		template < typename T >
		std::string operator()(T const& v)const{
			std::ostringstream os;
			print_if_supported(os, v);
			return os.str();
		}
	};

	constexpr auto default_value_help_generator_fn =
		default_value_help_fn< default_value_help_generator >();


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
			default_value_help_fn< HelpFn > const& help_fn
				= default_value_help_generator_fn
		)noexcept(
			std::is_nothrow_default_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_()
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			default_value_help_fn< HelpFn >&& help_fn
		)noexcept(
			std::is_nothrow_default_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_()
			, help_fn(std::move(help_fn)) {}

		constexpr default_value_fn(
			Fn const& fn,
			default_value_help_fn< HelpFn > const& help_fn
				= default_value_help_generator_fn
		)noexcept(
			std::is_nothrow_copy_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_(fn)
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			Fn const& fn,
			default_value_help_fn< HelpFn >&& help_fn
		)noexcept(
			std::is_nothrow_copy_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_(fn)
			, help_fn(std::move(help_fn)) {}

		constexpr default_value_fn(
			Fn&& fn,
			default_value_help_fn< HelpFn > const& help_fn
				= default_value_help_generator_fn
		)noexcept(
			std::is_nothrow_move_constructible_v< Fn > &&
			std::is_nothrow_copy_constructible_v< HelpFn >
		)
			: fn_(std::move(fn))
			, help_fn(help_fn) {}

		constexpr default_value_fn(
			Fn&& fn,
			default_value_help_fn< HelpFn >&& help_fn
		)noexcept(
			std::is_nothrow_move_constructible_v< Fn > &&
			std::is_nothrow_move_constructible_v< HelpFn >
		)
			: fn_(std::move(fn))
			, help_fn(std::move(help_fn)) {}


		template < typename T, typename Accessory >
		static constexpr bool is_invocable_v()noexcept{
			return
				std::is_invocable_v< Fn const, hana::basic_type< T >,
					Accessory > ||
				std::is_invocable_v< Fn const, hana::basic_type< T > > ||
				std::is_invocable_v< Fn const >;
		}

		template < typename T, typename Accessory >
		static constexpr auto invoke_result_type()noexcept{
			static_assert(is_invocable_v< T, Accessory >(),
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R is void or convertible to T"
			);

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
							hana::basic_type< T >, Accessory > >;
					}
				}();

			auto const is_result_valid =
				hana::traits::is_void(type) ||
				hana::traits::is_convertible(type, hana::type_c< T >);
			static_assert(is_result_valid,
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R must be void or convertible to T"
			);

			return type;
		}

		/// \brief true if correctly invocable and return type void,
		///        false otherwise
		template < typename T, typename Accessory >
		static constexpr auto is_void_r(
			hana::basic_type< T >, Accessory
		)noexcept{
			return hana::traits::is_void(invoke_result_type< T, Accessory >());
		}

		template < typename T, typename Accessory >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(is_invocable_v< T, Accessory >(),
				"Wrong function signature, expected one of:\n"
				"  R function()\n"
				"  R function(hana::basic_type< T > type)\n"
				"  R function(hana::basic_type< T > type, auto accessory)\n"
				"where R is void or convertible to T"
			);

			if constexpr(std::is_invocable_v< Fn const >){
				return std::is_nothrow_invocable_v< Fn const >;
			}else if constexpr(
				std::is_invocable_v< Fn const, hana::basic_type< T > >
			){
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T > >;
			}else{
				return std::is_nothrow_invocable_v< Fn const,
					hana::basic_type< T >, Accessory >;
			}
		}

		/// \brief Generate a value via the user defined function
		template < typename T, typename Accessory >
		T operator()(
			std::string_view parameter_name,
			hana::basic_type< T > type,
			Accessory accessory
		)const noexcept(calc_noexcept< T, Accessory >()){
			return accessory.log(
				[parameter_name](logsys::stdlogb& os, T const* value){
					os << "parameter(" << parameter_name << ") ";
					if(value){
						os << "generated default value: ";
						print_if_supported(os, *value);
					}else{
						os << "no default value generated";
					}
					os << " [" << ct_pretty_name< T >() << "]";
				}, [&]()noexcept(calc_noexcept< T, Accessory >())->T{
					if constexpr(std::is_invocable_v< Fn const >){
						return std::invoke(fn_);
					}else if constexpr(
						std::is_invocable_v< Fn const, hana::basic_type< T > >
					){
						return std::invoke(fn_, type);
					}else{
						return std::invoke(fn_, type, accessory);
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
