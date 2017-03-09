#include <disposer/input.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;


struct M1{
	decltype(io_list(
		in("test1"_s, hana::type_c< int >)
	)) io;
};

struct M2{
	decltype(io_list(
		in("test1"_s, hana::type_c< int >),
		in("test2"_s, hana::type_c< char >),
		in("test3"_s, hana::type_c< float >)
	)) io;
};

constexpr auto types = hana::tuple_t< int, char, float >;

struct M3{
	decltype(io_list(
		in("test1"_s, types)
	)) io;
};


constexpr auto types_set = hana::to_set(types);

struct M4{
	decltype(io_list(
		in("test1"_s, types_set)
	)) io;
};


struct M5{
	decltype(io_list(
		in("test1"_s, hana::type_c< int >),
		in("test2"_s, types),
		in("test3"_s, types_set)
	)) io;
};



int main(){
	M1 m1;
	static_assert(std::is_same_v< decltype(m1.io),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int > >
			> >,
			hana::type< hana::map<> >
		> >);

	M2 m2;
	static_assert(std::is_same_v< decltype(m2.io),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_s), char > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_s), float > >
			> >,
			hana::type< hana::map<> >
		> >);

	M3 m3;
	static_assert(std::is_same_v< decltype(m3.io),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int, char, float > >
			> >,
			hana::type< hana::map<> >
		> >);

	M4 m4;
	static_assert(std::is_same_v< decltype(m4.io),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int, char, float > >
			> >,
			hana::type< hana::map<> >
		> >);

	M5 m5;
	static_assert(std::is_same_v< decltype(m5.io),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_s), int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_s), int, char, float > >
			> >,
			hana::type< hana::map<> >
		> >);
}
