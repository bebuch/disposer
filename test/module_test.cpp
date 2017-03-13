#include <disposer/module.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	constexpr auto m1 = module("module1"_s,
			in("test1"_in, hana::type_c< int >),
			out("test1"_out, hana::type_c< int >)
		);
	static_assert(std::is_same_v< decltype(m1),
		hana::type< disposer::module<
			decltype("module1"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), int > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), int > >
			>
		> > const >);

	constexpr auto m2 = module("module2"_s,
			out("test1"_out, hana::type_c< int >),
			in("test1"_in, hana::type_c< int >),
			out("test2"_out, hana::type_c< char >),
			out("test3"_out, hana::type_c< float >),
			in("test2"_in, hana::type_c< char >),
			in("test3"_in, hana::type_c< float >)
		);
	static_assert(std::is_same_v< decltype(m2),
		hana::type< disposer::module<
			decltype("module2"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), char > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), char > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), float > >
			>
		> > const >);

	constexpr auto m3 = module("module3"_s,
			out("test1"_out, types),
			in("test1"_in, types)
		);
	static_assert(std::is_same_v< decltype(m3),
		hana::type< disposer::module<
			decltype("module3"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), int, char, float > >
			>
		> > const >);

	constexpr auto m4 = module("module4"_s,
			in("test1"_in, types_set),
			out("test1"_out, types_set)
		);
	static_assert(std::is_same_v< decltype(m4),
		hana::type< disposer::module<
			decltype("module4"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), int, char, float > >
			>
		> > const >);

	constexpr auto m5 = module("module5"_s,
			out("test1"_out, hana::type_c< int >),
			in("test1"_in, hana::type_c< int >),
			in("test2"_in, types),
			in("test3"_in, types_set),
			out("test2"_out, types),
			out("test3"_out, types_set)
		);
	static_assert(std::is_same_v< decltype(m5),
		hana::type< disposer::module<
			decltype("module5"_s),
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), int, char, float > >
			>,
			hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), int, char, float > >
			>
		> > const >);

}
