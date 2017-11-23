#include <disposer/core/module.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;
using namespace disposer::literals;

struct exec{
	void operator()()const{}
};


output< decltype("o1"_out), int > o1{1};
input< decltype("i1"_in), long, false > i1{nullptr};
input< decltype("i2"_in), int, true > i2{&o1};
parameter< decltype("p1"_param), int > p1{5};
parameter< decltype("p2"_param), char > p2{'x'};
output< decltype("o2"_out), bool > o2{0};

std::string const chain_name = "chain";
std::string const m_type_name = "type";
std::size_t const number = 4;

exec_fn< exec > e{};
module_init_fn< void > s{};

output_map_type map{{&o1, nullptr}};

constexpr auto t = hana::true_c;


BOOST_AUTO_TEST_CASE(test_1){
	auto list = iops_ref{};
	module m(type_list<>{dimension_list{}}, chain_name, m_type_name, number,
		std::move(list).flat(), s, e, t, optional_component< void >{});
	static_assert(std::is_same_v< decltype(m),
		module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		> >);

	auto e = m.make_exec_module(0, 0, map);
	BOOST_TEST((dynamic_cast< exec_module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		>* >(e.get()) != nullptr));

	e->exec();
	e->cleanup();
}

BOOST_AUTO_TEST_CASE(test_2){
	auto list = iops_ref{
		std::move(i2), iops_ref{std::move(i1), iops_ref{}}};
	module m(type_list<>{dimension_list{}}, chain_name, m_type_name, number,
		std::move(list).flat(), s, e, t, optional_component< void >{});
	static_assert(std::is_same_v< decltype(m), module<
			type_list<>,
			hana::tuple<
				input< decltype("i1"_in), long, false >,
				input< decltype("i2"_in), int, true >
			>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		> >);

	auto e = m.make_exec_module(0, 0, map);
	BOOST_TEST((dynamic_cast< exec_module<
			type_list<>,
			hana::tuple<
				input< decltype("i1"_in), long, false >,
				input< decltype("i2"_in), int, true >
			>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		>* >(e.get()) != nullptr));

	e->exec();
	e->cleanup();
}

BOOST_AUTO_TEST_CASE(test_3){
	auto list = iops_ref{
		std::move(o2), iops_ref{std::move(o1), iops_ref{}}};
	module m(type_list<>{dimension_list{}}, chain_name, m_type_name, number,
		std::move(list).flat(), s, e, t, optional_component< void >{});
	static_assert(std::is_same_v< decltype(m), module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<
				output< decltype("o1"_out), int >,
				output< decltype("o2"_out), bool >
			>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		> >);

	auto e = m.make_exec_module(0, 0, map);
	BOOST_TEST((dynamic_cast< exec_module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<
				output< decltype("o1"_out), int >,
				output< decltype("o2"_out), bool >
			>,
			hana::tuple<>,
			void,
			exec,
			true,
			void
		>* >(e.get()) != nullptr));

	e->exec();
	e->cleanup();
}

BOOST_AUTO_TEST_CASE(test_4){
	auto list = iops_ref{
		std::move(p2), iops_ref{std::move(p1), iops_ref{}}};
	module m(type_list<>{dimension_list{}}, chain_name, m_type_name, number,
		std::move(list).flat(), s, e, t, optional_component< void >{});
	static_assert(std::is_same_v< decltype(m), module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			void,
			exec,
			true,
			void
		> >);

	auto e = m.make_exec_module(0, 0, map);
	BOOST_TEST((dynamic_cast< exec_module<
			type_list<>,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			void,
			exec,
			true,
			void
		>* >(e.get()) != nullptr));

	e->exec();
	e->cleanup();
}

BOOST_AUTO_TEST_CASE(test_5){
	auto list = iops_ref{std::move(o2), iops_ref{std::move(p2),
		iops_ref{std::move(p1), iops_ref{std::move(i2),
		iops_ref{std::move(i1), iops_ref{std::move(o1), iops_ref{}}}}}}};
	module m(type_list<>{dimension_list{}}, chain_name, m_type_name, number,
		std::move(list).flat(), s, e, t, optional_component< void >{});
	static_assert(std::is_same_v< decltype(m), module<
			type_list<>,
			hana::tuple<
				input< decltype("i1"_in), long, false >,
				input< decltype("i2"_in), int, true >
			>,
			hana::tuple<
				output< decltype("o1"_out), int >,
				output< decltype("o2"_out), bool >
			>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			void,
			exec,
			true,
			void
		> >);

	auto e = m.make_exec_module(0, 0, map);
	BOOST_TEST((dynamic_cast< exec_module<
			type_list<>,
			hana::tuple<
				input< decltype("i1"_in), long, false >,
				input< decltype("i2"_in), int, true >
			>,
			hana::tuple<
				output< decltype("o1"_out), int >,
				output< decltype("o2"_out), bool >
			>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			void,
			exec,
			true,
			void
		>* >(e.get()) != nullptr));

	e->exec();
	e->cleanup();
}
