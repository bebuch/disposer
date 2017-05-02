#include <disposer/module.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;
constexpr auto types_set = hana::to_set(types);


void check_impl(std::string_view name, bool enabled, bool expected){
	if(enabled == expected){
		std::cout << " \033[0;32msuccess:\033[0m ";
	}else{
		std::cout << " \033[0;31mfail:\033[0m ";
	}
	std::cout << name << " = " << std::boolalpha << expected << "\n";
}

template < typename IO >
void check(IO& io, bool expected){
	check_impl(io.name, io.is_enabled(), expected);
}

template < typename IO, typename Type >
void check(IO& io, Type const& type, bool expected){
	check_impl(io.name, io.is_enabled(type), expected);
}


int main(){
	using ident =
		decltype(hana::typeid_(hana::template_< disposer::self_t >))::type;

	constexpr auto m1 = "module1"_module(
			"test1"_in(hana::type_c< int >),
			"test1"_out(hana::type_c< int >)
		);
	static_assert(std::is_same_v< decltype(m1),
		hana::type< disposer::module<
			decltype("module1"_module),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >
			>,
			hana::map<>
		> > const >);
	typename decltype(m1)::type mv1("chain", 0);
	auto& mv1i = mv1("test1"_in);
	auto& mv1o = mv1("test1"_out); (void)mv1o;

	check(mv1i, false);
	check(mv1i, hana::type_c< int >, false);
// 	check(mv1o, false);
// 	check(mv1o, hana::type_c< int >, false);

	constexpr auto m2 = "module2"_module(
			"test1"_out(hana::type_c< int >),
			"test1"_in(hana::type_c< int >),
			"test2"_out(hana::type_c< char >),
			"test3"_out(hana::type_c< float >),
			"test2"_in(hana::type_c< char >),
			"test3"_in(hana::type_c< float >)
		);
	static_assert(std::is_same_v< decltype(m2),
		hana::type< disposer::module<
			decltype("module2"_module),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), ident, char > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), ident, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), ident, char > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), ident, float > >
			>,
			hana::map<>
		> > const >);

	constexpr auto m3 = "module3"_module(
			"test1"_out(types),
			"test1"_in(types)
		);
	static_assert(std::is_same_v< decltype(m3),
		hana::type< disposer::module<
			decltype("module3"_module),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int, char, float > >
			>,
			hana::map<>
		> > const >);

	constexpr auto m4 = "module4"_module(
			"test1"_in(types_set),
			"test1"_out(types_set)
		);
	static_assert(std::is_same_v< decltype(m4),
		hana::type< disposer::module<
			decltype("module4"_module),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int, char, float > >
			>,
			hana::map<>
		> > const >);

	constexpr auto m5 = "module5"_module(
			"test1"_out(hana::type_c< int >),
			"test1"_in(hana::type_c< int >),
			"test2"_in(types),
			"test3"_in(types_set),
			"test2"_out(types),
			"test3"_out(types_set)
		);
	static_assert(std::is_same_v< decltype(m5),
		hana::type< disposer::module<
			decltype("module5"_module),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), ident, int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), ident, int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), ident, int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), ident, int, char, float > >
			>,
			hana::map<>
		> > const >);

}
