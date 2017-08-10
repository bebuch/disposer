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

#include "output_info.hpp"

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <boost/hana/core/is_a.hpp>
#include <boost/hana/count_if.hpp>
#include <boost/hana/string.hpp>
#include <boost/hana/all_of.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/map.hpp>


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

			static_assert(std::is_nothrow_invocable_v< Fn, type >);

			using result = decltype(std::declval< Fn >()(type{}));

			static_assert(hana::is_a< hana::type_tag, result >());

			return result{};
		}
	};


	struct none{
		template < typename T >
		constexpr auto operator()(hana::basic_type< T > type)const noexcept{
			return type;
		}
	};

	constexpr auto no_type_transform = type_transform_fn< none >{};


	template < template < typename > typename Template >
	struct wrap_in_t{
		template < typename T >
		constexpr auto operator()(hana::basic_type< T >)const noexcept{
			return hana::type_c< Template< T > >;
		}
	};

	template < template < typename > typename Template >
	constexpr auto wrap_in = type_transform_fn< wrap_in_t< Template > >{};



	struct enable_fn_tag;

	template < typename Fn >
	class enable_fn{
	public:
		using hana_tag = enable_fn_tag;

		constexpr enable_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit enable_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit enable_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(
				std::is_invocable_r_v< bool, Fn const, Accessory const&,
					hana::basic_type< T > >,
				"Wrong function signature, expected: "
				"bool f(auto const& iop, hana::basic_type< T > type)"
			);

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< hana::basic_type< T > const >()
			));
		}

		/// \brief Operator for outputs
		template < typename Accessory, typename T >
		bool operator()(
			Accessory const& accessory,
			hana::basic_type< T > type
		)const noexcept(calc_noexcept< Accessory, T >()){
			return accessory.log(
				[](logsys::stdlogb& os, bool const* enabled){
					os << "type ";
					if(enabled){
						if(*enabled){
							os << "enabled";
						}else{
							os << "disabled";
						}
					}else{
						os << "enabling";
					}
					os << " ["
						<< type_index::type_id< T >().pretty_name() << "]";
				}, [&]()->bool{
					return std::invoke(fn_, accessory, type);
				});
		}

		/// \brief Operator for parameters
		template < typename Accessory, typename T >
		bool operator()(
			Accessory const& accessory,
			hana::basic_type< T > type,
			std::string_view name
		)const noexcept(calc_noexcept< Accessory, T >()){
			return accessory.log(
				[name](logsys::stdlogb& os, bool const* enabled){
					os << "type ";
					if(enabled){
						if(*enabled){
							os << "enabled";
						}else{
							os << "disabled";
						}
					}else{
						os << "enabling";
					}
					os << " " << name << " ["
						<< type_index::type_id< T >().pretty_name() << "]";
				}, [&]()->bool{
					return std::invoke(fn_, accessory, type);
				});
		}

	private:
		Fn fn_;
	};


	struct enable_always_t{
		template < typename Accessory, typename T >
		constexpr bool operator()(
			Accessory const&, hana::basic_type< T >
		)const noexcept{ return true; }
	};

	constexpr auto enable_always = enable_fn< enable_always_t >{};


	template < typename Name >
	struct enable_by_transformed_types_of_t{
		template < typename Accessory, typename Type >
		auto operator()(Accessory const& iop, Type type)const{
			auto const& other = iop(Name{});
			return other.is_enabled(other.type_transform(type));
		}
	};

	template < typename Name >
	constexpr auto enable_by_transformed_types_of(Name const&){
		return enable_fn(enable_by_transformed_types_of_t< Name >{});
	}


	template < typename Name >
	struct enable_by_types_of_t{
		template < typename Accessory, typename Type >
		auto operator()(Accessory const& iop, Type type)const{
			return iop(Name{}).is_subtype_enabled(type);
		}
	};

	template < typename Name >
	constexpr auto enable_by_types_of(Name const&){
		return enable_fn(enable_by_types_of_t< Name >{});
	}



	struct verify_connection_fn_tag;

	template < typename Fn >
	class verify_connection_fn{
	public:
		using hana_tag = verify_connection_fn_tag;

		constexpr verify_connection_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit verify_connection_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit verify_connection_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(
				std::is_invocable_v< Fn const, Accessory const&, bool >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, bool connected)"
			);

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< bool >()
			));
		}


		template < typename Accessory >
		void operator()(
			Accessory const& accessory,
			bool connected
		)const noexcept(calc_noexcept< Accessory >()){
			accessory.log([connected](logsys::stdlogb& os){
					os << "verified ";
					if(connected){
						os << "enabled";
					}else{
						os << "disabled";
					}
					os << " connection";
				}, [&]{ std::invoke(fn_, accessory, connected); });
		}

	private:
		Fn fn_;
	};


	struct required_t{
		template < typename Accessory >
		void operator()(Accessory const&, bool connected)const{
			if(!connected) throw std::logic_error("input is required");
		}
	};

	constexpr auto required = verify_connection_fn(required_t{});


	struct optional_t{
		template < typename Accessory >
		void operator()(Accessory const&, bool)const noexcept{}
	};

	constexpr auto optional = verify_connection_fn< optional_t >{};



	struct verify_type_fn_tag;

	template < typename Fn >
	class verify_type_fn{
	public:
		using hana_tag = verify_type_fn_tag;

		constexpr verify_type_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit verify_type_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit verify_type_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(
				std::is_invocable_v< Fn const, Accessory const&,
					hana::basic_type< T >, output_info const& >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, hana::basic_type< T > type, "
				"output_info const& info)"
			);

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< hana::basic_type< T > const >(),
				std::declval< output_info const& >()
			));
		}

		template < typename Accessory, typename T >
		void operator()(
			Accessory const& accessory,
			hana::basic_type< T > type,
			output_info const& info
		)const noexcept(calc_noexcept< Accessory, T >()){
			accessory.log(
				[&info, type](logsys::stdlogb& os){
					os << "verified ";
					if(info.is_enabled(type)){
						os << "enabled";
					}else{
						os << "disabled";
					}
					os << " type ["
						<< type_index::type_id< T >().pretty_name() << "]";
				}, [&]{ std::invoke(fn_, accessory, type, info); });
		}

	private:
		Fn fn_;
	};


	struct verify_type_always_t{
		template < typename Accessory, typename T >
		void operator()(
			Accessory const& /* accessory */,
			hana::basic_type< T > /*type*/,
			output_info const& /*info*/
		)const noexcept{}
	};

	auto constexpr verify_type_always =
		verify_type_fn< verify_type_always_t >{};



	struct verify_value_fn_tag;

	template < typename Fn >
	class verify_value_fn{
	public:
		using hana_tag = verify_value_fn_tag;

		constexpr verify_value_fn()
			noexcept(std::is_nothrow_default_constructible_v< Fn >)
			: fn_() {}

		constexpr explicit verify_value_fn(Fn const& fn)
			noexcept(std::is_nothrow_copy_constructible_v< Fn >)
			: fn_(fn) {}

		constexpr explicit verify_value_fn(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v< Fn >)
			: fn_(std::move(fn)) {}


		template < typename Accessory, typename T >
		static constexpr bool calc_noexcept()noexcept{
			static_assert(
				std::is_invocable_v< Fn const, Accessory const&, T const& >,
				"Wrong function signature, expected: "
				"void f(auto const& iop, auto const& value)"
			);

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< T const >()
			));
		}

		template < typename Accessory, typename T >
		void operator()(
			Accessory const& accessory,
			T const& value
		)const noexcept(calc_noexcept< Accessory, T >()){
			accessory.log(
				[](logsys::stdlogb& os){
					os << "verified value of type ["
						<< type_index::type_id< T >().pretty_name() << "]";
				}, [&]{ std::invoke(fn_, accessory, value); });
		}

	private:
		Fn fn_;
	};

	struct verify_value_always_t{
		template < typename Accessory, typename T >
		void operator()(
			Accessory const& /* accessory */,
			T const& /*value*/
		)const noexcept{}
	};

	auto constexpr verify_value_always =
		verify_value_fn< verify_value_always_t >{};



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

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< std::string_view >(),
				std::declval< hana::basic_type< T > >()
			));
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
				}, [&]()->T{
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
				return std::true_type{};
			}else{
				return std::false_type{};
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

			return noexcept(std::declval< Fn const >()(
				std::declval< Accessory const >(),
				std::declval< hana::basic_type< T > const >()
			));
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
				}, [&]()->T{
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
