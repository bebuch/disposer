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
#include <boost/hana/tuple.hpp>

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

	template < typename Fn >
	class type_transform_fn{
	public:
		static_assert(std::is_nothrow_default_constructible_v< Fn >);

		template < typename T >
		struct apply{
			using type = decltype(type_transform_fn{}(std::declval< T >()));
		};

		template < typename T >
		constexpr auto operator()(T const&)const noexcept{
			using type = std::conditional_t<
				hana::is_a< hana::type_tag, T >, T, hana::type< T > >;

// TODO: remove result_of-version as soon as libc++ supports invoke_result_t
#if __clang__
			static_assert(std::is_callable_v< Fn(type) >);
#else
			static_assert(std::is_nothrow_invocable_v< Fn, type >);
#endif

			auto result = Fn{}(type{});

			static_assert(hana::is_a< hana::type_tag >(result));

			return result;
		}
	};

	template < typename Fn >
	constexpr auto type_transform(Fn&&)noexcept{
		return type_transform_fn< std::remove_reference_t< Fn > >();
	}


	struct enable_always{
		template < typename IOP_List, typename T >
		constexpr bool operator()(
			IOP_List const& /* iop_list */,
			hana::basic_type< T > /*type*/
		)const noexcept{
			return true;
		}
	};

	template < typename Fn >
	class enable_fn{
	public:
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
		constexpr bool operator()(
			IOP_List const& iop_list,
			hana::basic_type< T > type
		)const noexcept(noexcept(std::declval< Fn const >()(iop_list, type))){
			return fn_(iop_list, type);
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


	struct connection_verify_always{
		template < typename IOP_List >
		constexpr void operator()(
			IOP_List const& /* iop_list */,
			bool /*connected*/
		)const noexcept{}
	};

	template < typename Fn >
	class connection_verify_fn{
	public:
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
		constexpr void operator()(
			IOP_List const& iop_list,
			bool connected
		)const noexcept(noexcept(
			std::declval< Fn const >()(iop_list, connected)
		)){
			fn_(iop_list, connected);
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


	struct type_verify_always{
		template < typename IOP_List, typename T >
		constexpr void operator()(
			IOP_List const& /* iop_list */,
			hana::basic_type< T > /*type*/,
			output_info const& /*info*/
		)const noexcept{}
	};

	template < typename Fn >
	class type_verify_fn{
	public:
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
		constexpr void operator()(
			IOP_List const& iop_list,
			hana::basic_type< T > type,
			output_info const& info
		)const noexcept(noexcept(
			std::declval< Fn const >()(iop_list, type, info)
		)){
			fn_(iop_list, type, info);
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

	template < typename Fn >
	class value_verify_fn{
	public:
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
		constexpr void operator()(
			IOP_List const& iop_list,
			T const& value
		)const noexcept(noexcept(std::declval< Fn const >()(iop_list, value))){
			fn_(iop_list, value);
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
		template < typename T >
		T operator()(
			std::string_view  value ,
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

	template < typename Fn >
	class parser_fn{
	public:
		constexpr parser_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		explicit constexpr parser_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		explicit constexpr parser_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename T >
		constexpr T operator()(
			std::string_view value,
			hana::basic_type< T > type
		)const noexcept(noexcept(std::declval< Fn const >()(value, type))){
			return fn_(value, type);
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


}


namespace boost::hana{


    template < typename Fn >
    struct Metafunction< disposer::type_transform_fn< Fn > >{
        static constexpr bool value = true;
    };


}


#endif
