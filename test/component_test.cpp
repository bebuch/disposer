#include <disposer/core/component.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;
using namespace disposer::literals;


parameter< decltype("p1"_param), int > p1{5};
parameter< decltype("p2"_param), char > p2{'x'};

struct init_fn{
	auto operator()(){
		return 0;
	}
};

component_init_fn< init_fn > s{};


BOOST_AUTO_TEST_CASE(test_0){
	auto list = iops_ref{};
	component m(type_list<>{dimension_list{}}, std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(m),
		component<
			type_list<>,
			hana::tuple<>,
			init_fn
		> >);

	auto e = m.make_exec_component(0, map);
	BOOST_TEST((dynamic_cast< exec_component<
			type_list<>,
			hana::tuple<>,
			init_fn
		>* >(e.get()) != nullptr));
}

BOOST_AUTO_TEST_CASE(test_1){
	auto list = iops_ref{std::move(p1), iops_ref{}};
	component m(type_list<>{dimension_list{}}, std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(m), component<
			type_list<>,
			hana::tuple< parameter< decltype("p1"_param), int > >,
			init_fn
		> >);

	auto e = m.make_exec_component(0, map);
	BOOST_TEST((dynamic_cast< exec_component<
			type_list<>,
			hana::tuple< parameter< decltype("p1"_param), int > >,
			init_fn
		>* >(e.get()) != nullptr));
}

BOOST_AUTO_TEST_CASE(test_2){
	auto list = iops_ref{
		std::move(p2), iops_ref{std::move(p1), iops_ref{}}};
	component m(type_list<>{dimension_list{}}, std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(m), component<
			type_list<>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			init_fn
		> >);

	auto e = m.make_exec_component(0, map);
	BOOST_TEST((dynamic_cast< exec_component<
			type_list<>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			init_fn
		>* >(e.get()) != nullptr));
}
