#include <disposer/output.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using ident =
		decltype(hana::typeid_(hana::template_< disposer::self_t >))::type;

	static constexpr auto iops = hana::make_tuple();
	static constexpr auto get_object =
		disposer::iop_list< hana::tuple<> >(iops);

	{
		auto const maker = "v"_out(hana::type_c< int >);

		static_assert(std::is_same_v< decltype(maker),
			disposer::output_maker<
				decltype("v"_out),
				disposer::output< decltype("v"_out), ident, int >,
				disposer::enable_all
			> const >);

		auto object = maker(get_object);

		static_assert(std::is_same_v< decltype(object),
			disposer::output< decltype("v"_out), ident, int > >);

		object.put(0);
	}

	{
		auto const maker = "v"_out(types);

		static_assert(std::is_same_v< decltype(maker),
			disposer::output_maker<
				decltype("v"_out),
				disposer::output< decltype("v"_out), ident, int, float >,
				disposer::enable_all
			> const >);

		auto object = maker(get_object);

		static_assert(std::is_same_v< decltype(object),
			disposer::output< decltype("v"_out), ident, int, float > >);

		object.put(3);
		object.put(3.f);
	}

	{
		auto const maker = "v"_out(types_set);

		static_assert(std::is_same_v< decltype(maker),
			disposer::output_maker<
				decltype("v"_out),
				disposer::output< decltype("v"_out), ident, int, float >,
				disposer::enable_all
			> const >);

		auto object = maker(get_object);

		static_assert(std::is_same_v< decltype(object),
			disposer::output< decltype("v"_out), ident, int, float > >);

		object.put(3);
		object.put(3.f);
	}

// 	constexpr auto m2 = disposer::separate_module_config_lists(
// 			"test1"_out(hana::type_c< int >),
// 			"test2"_out(hana::type_c< char >),
// 			"test3"_out(hana::type_c< float >)
// 		);
// 	static_assert(std::is_same_v< decltype(m2),
// 		hana::tuple<
// 			hana::type< hana::map<> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_out), ident, int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::output< decltype("test2"_out), ident, char > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::output< decltype("test3"_out), ident, float > >
// 			> >,
// 			hana::type< hana::map<> >
// 		> const >);
//
// 	constexpr auto m3 = disposer::separate_module_config_lists(
// 			"test1"_out(types)
// 		);
// 	static_assert(std::is_same_v< decltype(m3),
// 		hana::tuple<
// 			hana::type< hana::map<> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_out), ident, int, char, float > >
// 			> >,
// 			hana::type< hana::map<> >
// 		> const >);
//
// 	constexpr auto m4 = disposer::separate_module_config_lists(
// 			"test1"_out(types_set)
// 		);
// 	static_assert(std::is_same_v< decltype(m4),
// 		hana::tuple<
// 			hana::type< hana::map<> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_out), ident, int, char, float > >
// 			> >,
// 			hana::type< hana::map<> >
// 		> const >);
//
// 	constexpr auto m5 = disposer::separate_module_config_lists(
// 			"test1"_out(hana::type_c< int >),
// 			"test2"_out(types),
// 			"test3"_out(types_set)
// 		);
// 	static_assert(std::is_same_v< decltype(m5),
// 		hana::tuple<
// 			hana::type< hana::map<> >,
// 			hana::type< hana::map<
// 				hana::pair< decltype("test1"_s),
// 					disposer::output< decltype("test1"_out), ident, int > >,
// 				hana::pair< decltype("test2"_s),
// 					disposer::output< decltype("test2"_out), ident, int, char, float > >,
// 				hana::pair< decltype("test3"_s),
// 					disposer::output< decltype("test3"_out), ident, int, char, float > >
// 			> >,
// 			hana::type< hana::map<> >
// 		> const >);
}
