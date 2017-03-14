#include <disposer/output.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using ident = meta_identity_t;

	constexpr auto m1 = disposer::make_io_lists(
			"test1"_out(hana::type_c< int >)
		);
	static_assert(std::is_same_v< decltype(m1),
		hana::pair<
			hana::type< hana::map<> >,
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >
			> >
		> const >);

	constexpr auto m2 = disposer::make_io_lists(
			"test1"_out(hana::type_c< int >),
			"test2"_out(hana::type_c< char >),
			"test3"_out(hana::type_c< float >)
		);
	static_assert(std::is_same_v< decltype(m2),
		hana::pair<
			hana::type< hana::map<> >,
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), ident, char > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), ident, float > >
			> >
		> const >);

	constexpr auto m3 = disposer::make_io_lists(
			"test1"_out(types)
		);
	static_assert(std::is_same_v< decltype(m3),
		hana::pair<
			hana::type< hana::map<> >,
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int, char, float > >
			> >
		> const >);

	constexpr auto m4 = disposer::make_io_lists(
			"test1"_out(types_set)
		);
	static_assert(std::is_same_v< decltype(m4),
		hana::pair<
			hana::type< hana::map<> >,
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int, char, float > >
			> >
		> const >);

	constexpr auto m5 = disposer::make_io_lists(
			"test1"_out(hana::type_c< int >),
			"test2"_out(types),
			"test3"_out(types_set)
		);
	static_assert(std::is_same_v< decltype(m5),
		hana::pair<
			hana::type< hana::map<> >,
			hana::type< hana::map<
				hana::pair< decltype("test1"_s),
					disposer::output< decltype("test1"_out), ident, int > >,
				hana::pair< decltype("test2"_s),
					disposer::output< decltype("test2"_out), ident, int, char, float > >,
				hana::pair< decltype("test3"_s),
					disposer::output< decltype("test3"_out), ident, int, char, float > >
			> >
		> const >);
}
