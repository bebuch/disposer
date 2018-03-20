#include <disposer/core/component.hpp>

#define BOOST_TEST_MODULE disposer component_test
#include <boost/test/included/unit_test.hpp>


using namespace disposer;
using namespace disposer::literals;


parameter< decltype("p1"_param), int > p1{5};
parameter< decltype("p2"_param), char > p2{'x'};

struct init_fn{
	auto operator()()const{
		return 0;
	}
};

component_init_fn< init_fn > s{};

std::string const name = "component";
std::string const type = "type";

disposer::system d;


BOOST_AUTO_TEST_CASE(test_0){
	auto list = iops_ref{};
	component c(type_list<>{dimension_list{}}, name, type, d,
		std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(c),
		component<
			type_list<>,
			hana::tuple<>,
			init_fn
		> >);
}

BOOST_AUTO_TEST_CASE(test_1){
	auto list = iops_ref{std::move(p1), iops_ref{}};
	component c(type_list<>{dimension_list{}}, name, type, d,
		std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(c), component<
			type_list<>,
			hana::tuple< parameter< decltype("p1"_param), int > >,
			init_fn
		> >);
}

BOOST_AUTO_TEST_CASE(test_2){
	auto list = iops_ref{
		std::move(p2), iops_ref{std::move(p1), iops_ref{}}};
	component c(type_list<>{dimension_list{}}, name, type, d,
		std::move(list).flat(), s);
	static_assert(std::is_same_v< decltype(c), component<
			type_list<>,
			hana::tuple<
				parameter< decltype("p1"_param), int >,
				parameter< decltype("p2"_param), char >
			>,
			init_fn
		> >);
}
