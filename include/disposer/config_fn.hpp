//-----------------------------------------------------------------------------
// Copyright (c) 2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__config_fn__hpp_INCLUDED_
#define _disposer__config_fn__hpp_INCLUDED_

#include <type_traits>

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/all_of.hpp>
#include <boost/hana/string.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/map.hpp>

#include <logsys/stdlogb.hpp>

#include "output_info.hpp"


namespace disposer{


	struct no_transform{
		template < typename T >
		constexpr auto operator()(hana::basic_type< T > type)const noexcept{
			return type;
		}
	};

	template < template < typename > typename Template >
	struct template_transform{
		template < typename T >
		constexpr auto operator()(hana::basic_type< T >)const noexcept{
			return hana::type_c< Template< T > >;
		}
	};


	struct type_transform_fn_tag;

	template < typename Fn >
	class type_transform_fn{
	public:
		using hana_tag = type_transform_fn_tag;


		template < typename T >
		struct apply{
			using type = typename decltype(std::declval< type_transform_fn >()
				(std::declval< T >()))::type;
		};

		template < typename T >
		constexpr auto operator()(T const&)const noexcept{
			using type = std::conditional_t<
				hana::is_a< hana::type_tag, T >, T, hana::type< T > >;

// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			static_assert(std::is_nothrow_callable_v< Fn(type) >);
#else
			static_assert(std::is_nothrow_invocable_v< Fn, type >);
#endif

			using result = decltype(std::declval< Fn >()(type{}));

			static_assert(hana::is_a< hana::type_tag, result >());

			return result{};
		}
	};

	template < typename Fn >
	constexpr auto type_transform(Fn&&)noexcept{
		return type_transform_fn< std::remove_reference_t< Fn > >();
	}

	template < template < typename > typename Template >
	constexpr auto template_transform_c
		= type_transform(template_transform< Template >{});


	struct enable_always{
		template < typename IOP_List, typename T >
		constexpr bool operator()(
			IOP_List const& /* iop_list */,
			hana::basic_type< T > /*type*/
		)const noexcept{
			return true;
		}
	};

	struct enable_fn_tag;

	template < typename Fn >
	class enable_fn{
	public:
		using hana_tag = enable_fn_tag;

		constexpr enable_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr enable_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr enable_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List, typename T >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&,
					hana::basic_type< T >), bool >,
				"Wrong function signature, expected: "
				"bool f(auto const& iop, hana::basic_type< T > type)"
			);
#else
			static_assert(
				std::is_invocable_r_v< bool, Fn const, IOP_List const&,
					hana::basic_type< T > >,
				"Wrong function signature, expected: "
				"bool f(auto const& iop, hana::basic_type< T > type)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >(),
				std::declval< hana::basic_type< T > const >()
			));
		}

		template < typename IOP_List, typename T >
		constexpr bool operator()(
			IOP_List const& iop_list,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< IOP_List, T >()){
			bool enable = false;
			iop_list.log([&enable](logsys::stdlogb& os){
					os << "enable = ";
					if(enable){
						os << "true";
					}else{
						os << "false";
					}
				},
				[&]{ enable = fn_(iop_list, type); });
			return enable;
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto enable(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return enable_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}

	template < typename IOP_Name >
	struct enable_by_transformed_types_of_t{
		template < typename IOP_List, typename Type >
		constexpr auto operator()(IOP_List const& iop, Type type)const{
			auto const& other = iop(IOP_Name{});
			return other.is_enabled(other.type_transform(type));
		}
	};

	template < typename IOP_Name >
	constexpr auto enable_by_transformed_types_of(IOP_Name const&){
		return enable(enable_by_transformed_types_of_t< IOP_Name >{});
	}

	template < typename IOP_Name >
	struct enable_by_types_of_t{
		template < typename IOP_List, typename Type >
		constexpr auto operator()(IOP_List const& iop, Type type)const{
			return iop(IOP_Name{}).is_subtype_enabled(type);
		}
	};

	template < typename IOP_Name >
	constexpr auto enable_by_types_of(IOP_Name const&){
		return enable(enable_by_types_of_t< IOP_Name >{});
	}


	struct connection_verify_always{
		template < typename IOP_List >
		constexpr void operator()(
			IOP_List const& /* iop_list */,
			bool /*connected*/
		)const noexcept{}
	};

	struct connection_verify_fn_tag;

	template < typename Fn >
	class connection_verify_fn{
	public:
		using hana_tag = connection_verify_fn_tag;

		constexpr connection_verify_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr connection_verify_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr connection_verify_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&, bool) >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, bool connected)"
			);
#else
			static_assert(
				std::is_invocable_v< Fn const, IOP_List const&, bool >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, bool connected)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >(),
				std::declval< bool >()
			));
		}


		template < typename IOP_List >
		constexpr void operator()(
			IOP_List const& iop_list,
			bool connected
		)const noexcept(calc_noexcept< IOP_List >()){
			iop_list.log([connected](logsys::stdlogb& os){
					os << "connection_verify with connected = ";
					if(connected){
						os << "true";
					}else{
						os << "false";
					}
				},
				[&]{ fn_(iop_list, connected); });
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto connection_verify(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return connection_verify_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}

	constexpr auto required = connection_verify([](auto const&, bool connected){
			if(!connected) throw std::logic_error("input is required");
		});


	struct type_verify_always{
		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& /* iop_list */,
			hana::basic_type< T > /*type*/,
			output_info const& /*info*/
		)const noexcept{}
	};

	struct type_verify_fn_tag;

	template < typename Fn >
	class type_verify_fn{
	public:
		using hana_tag = type_verify_fn_tag;

		constexpr type_verify_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr type_verify_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr type_verify_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List, typename T >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&,
					hana::basic_type< T >, output_info const&) >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, hana::basic_type< T > type, "
				"output_info const& info)"
			);
#else
			static_assert(
				std::is_invocable_v< Fn const, IOP_List const&,
					hana::basic_type< T >, output_info const& >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, hana::basic_type< T > type, "
				"output_info const& info)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >(),
				std::declval< hana::basic_type< T > const >(),
				std::declval< output_info const& >()
			));
		}

		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& iop_list,
			hana::basic_type< T > type,
			output_info const& info
		)const noexcept(calc_noexcept< IOP_List, T >()){
			iop_list.log([](logsys::stdlogb& os){ os << "type_verify"; },
				[&]{ fn_(iop_list, type, info); });
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto type_verify(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return type_verify_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}


	struct value_verify_always{
		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& /* iop_list */,
			T const& /*value*/
		)const noexcept{}
	};

	struct value_verify_fn_tag;

	template < typename Fn >
	class value_verify_fn{
	public:
		using hana_tag = value_verify_fn_tag;

		constexpr value_verify_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr value_verify_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr value_verify_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List, typename T >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&, T const&) >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, auto const& value)"
			);
#else
			static_assert(
				std::is_invocable_v< Fn const, IOP_List const&, T const& >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, auto const& value)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >(),
				std::declval< T const >()
			));
		}

		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& iop_list,
			T const& value
		)const noexcept(calc_noexcept< IOP_List, T >()){
			iop_list.log([](logsys::stdlogb& os){ os << "value_verify"; },
				[&]{ fn_(iop_list, value); });
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto value_verify(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return value_verify_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}


	struct stream_parser{
		template < typename IOP_List, typename T >
		T operator()(
			IOP_List const& /*iop_list*/,
			std::string_view value,
			hana::basic_type< T > type
		)const{
			if constexpr(type == hana::type_c< std::string >){
				return std::string(value);
			}else{
				std::istringstream is((std::string(value)));
				T result;
				if constexpr(std::is_same_v< T, bool >){
					is >> std::boolalpha;
				}
				is >> result;
				return result;
			}
		}
	};

	struct parser_fn_tag;

	template < typename Fn >
	class parser_fn{
	public:
		using hana_tag = parser_fn_tag;

		constexpr parser_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr parser_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr parser_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename IOP_List, typename T >
		static constexpr bool calc_noexcept()noexcept{
#if __clang__
			static_assert(
				std::is_callable_v< Fn const(IOP_List const&,
					std::string_view, hana::basic_type< T >), T >,
				"Wrong function signature, expected: "
				"T f(auto const& iop, std::string_view value, "
				"hana::basic_type< T > type)"
			);
#else
			static_assert(
				std::is_invocable_r_v< T, Fn const, IOP_List const&,
					std::string_view, hana::basic_type< T > >,
				"Wrong function signature, expected: "
				"T f(auto const& iop, std::string_view value, "
				"hana::basic_type< T > type)"
			);
#endif
			return noexcept(std::declval< Fn const >()(
				std::declval< IOP_List const >(),
				std::declval< std::string_view >(),
				std::declval< hana::basic_type< T > >()
			));
		}

		template < typename IOP_List, typename T >
		constexpr T operator()(
			IOP_List const& iop_list,
			std::string_view value,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< IOP_List, T >()){
			return iop_list.log(
				[](logsys::stdlogb& os){ os << "parser"; },
				[&]{ return fn_(iop_list, value, type); });
		}

	private:
		Fn fn_;
	};

	template < typename Fn >
	constexpr auto parser(Fn&& fn)
		noexcept(std::is_nothrow_constructible_v< Fn, Fn&& >){
		return parser_fn< std::remove_reference_t< Fn > >(
			static_cast< Fn&& >(fn));
	}


	/// \brief Create a hana::tuple of hana::type's with a given hana::type or
	///        a hana::Sequence of hana::type's
	template < typename Types >
	constexpr auto to_typelist(Types const&)noexcept{
		if constexpr(hana::is_a< hana::type_tag, Types >){
			return hana::make_tuple(Types{});
		}else{
			static_assert(hana::Foldable< Types >::value);
			static_assert(hana::all_of(Types{}, hana::is_a< hana::type_tag >));
			return hana::to_tuple(Types{});
		}
	}


	struct default_values_tuple_tag;

	template < typename DefaultValuesTuple >
	struct default_values_tuple{
		using hana_tag = default_values_tuple_tag;

		DefaultValuesTuple values;
	};

	template < typename ... Value >
	constexpr auto default_values(Value&& ... value)noexcept{
		return default_values_tuple<
			decltype(hana::make_tuple(static_cast< Value&& >(value) ...)) >{
				hana::make_tuple(static_cast< Value&& >(value) ...)
			};
	}


	struct type_as_text_map_tag;

	template < typename TypeAsTextMap >
	struct type_as_text_map{
		using hana_tag = type_as_text_map_tag;
	};

	template < typename ... Pair >
	constexpr auto type_as_text(Pair&& ... pair)noexcept{
		static_assert((true && ... && hana::is_a< hana::pair_tag, Pair >),
			"all parameters must be hana::pair's");
		static_assert((true && ... && hana::is_a< hana::type_tag,
				decltype(hana::first(std::declval< Pair >())) >),
			"keys of all hana::pair's must be a hana::type");
		static_assert((true && ... && hana::is_a< hana::string_tag,
				decltype(hana::second(std::declval< Pair >())) >),
			"values of all hana::pair's must be a hana::string");
		return type_as_text_map<
			decltype(hana::make_map(static_cast< Pair&& >(pair) ...)) >{};
	}


	struct no_argument_tag;

	struct no_argument{
		using hana_tag = no_argument_tag;
	};


	template < typename Tuple, typename Predicate, typename Default >
	constexpr auto get_or_default(
		Tuple&& tuple,
		Predicate&& predicate,
		Default&& default_value
	){
		auto result = hana::find_if(
			static_cast< Tuple&& >(tuple), static_cast< Predicate >(predicate));
		auto found = result != hana::nothing;
		if constexpr(found){
			return *result;
		}else{
			return static_cast< Default&& >(default_value);
		}
	}


}


namespace boost::hana{


    template < typename Fn >
    struct Metafunction< disposer::type_transform_fn< Fn > >{
        static constexpr bool value = true;
    };


}


#endif
