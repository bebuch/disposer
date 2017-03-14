#include <disposer/input.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using ident =
		decltype(hana::typeid_(hana::template_< disposer::self_t >))::type;

	constexpr auto m1 = disposer::make_io_lists(
			"test1"_in(hana::type_c< int >)
		);
	static_assert(std::is_same_v< decltype(m1),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >
			> >,
			hana::type< hana::map<> >
		> const >);

	constexpr auto m2 = disposer::make_io_lists(
			"test1"_in(hana::type_c< int >),
			"test2"_in(hana::type_c< char >),
			"test3"_in(hana::type_c< float >)
		);
	static_assert(std::is_same_v< decltype(m2),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), ident, char > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), ident, float > >
			> >,
			hana::type< hana::map<> >
		> const >);

	constexpr auto m3 = disposer::make_io_lists(
			"test1"_in(types)
		);
	static_assert(std::is_same_v< decltype(m3),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int, char, float > >
			> >,
			hana::type< hana::map<> >
		> const >);

	constexpr auto m4 = disposer::make_io_lists(
			"test1"_in(types_set)
		);
	static_assert(std::is_same_v< decltype(m4),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int, char, float > >
			> >,
			hana::type< hana::map<> >
		> const >);

	constexpr auto m5 = disposer::make_io_lists(
			"test1"_in(hana::type_c< int >),
			"test2"_in(types),
			"test3"_in(types_set)
		);
	static_assert(std::is_same_v< decltype(m5),
		hana::pair<
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::input< decltype("test1"_in), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::input< decltype("test2"_in), ident, int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::input< decltype("test3"_in), ident, int, char, float > >
			> >,
			hana::type< hana::map<> >
		> const >);
}
