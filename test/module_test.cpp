#include <disposer/module.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	// TODO: Check forgotten _s to make a hana string literal
	constexpr auto mx = module("module1", io_list());

	constexpr auto m1 = module("module1"_s,
		io_list(
			in("test1"_s, hana::type_c< int >),
			out("test1"_s, hana::type_c< int >)
		));
	static_assert(std::is_same_v< decltype(m1),
		hana::type< disposer::module<
			decltype("module1"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_s), int > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_s), int > >
			>
		> > const >);

// 	M2 m2;
// 	static_assert(std::is_same_v< decltype(m2.io),
// 		hana::pair<
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::input< decltype("test1"_s), int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::input< decltype("test2"_s), char > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::input< decltype("test3"_s), float > >
// 			> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_s), int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::output< decltype("test2"_s), char > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::output< decltype("test3"_s), float > >
// 			> >
// 		> >);
//
// 	M3 m3;
// 	static_assert(std::is_same_v< decltype(m3.io),
// 		hana::pair<
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::input< decltype("test1"_s), int, char, float > >
// 			> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_s), int, char, float > >
// 			> >
// 		> >);
//
// 	M4 m4;
// 	static_assert(std::is_same_v< decltype(m4.io),
// 		hana::pair<
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::input< decltype("test1"_s), int, char, float > >
// 			> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_s), int, char, float > >
// 			> >
// 		> >);
//
// 	M5 m5;
// 	static_assert(std::is_same_v< decltype(m5.io),
// 		hana::pair<
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::input< decltype("test1"_s), int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::input< decltype("test2"_s), int, char, float > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::input< decltype("test3"_s), int, char, float > >
// 			> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_s), int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::output< decltype("test2"_s), int, char, float > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::output< decltype("test3"_s), int, char, float > >
// 			> >
// 		> >);
}
