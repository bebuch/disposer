#include <disposer/parameter.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, long, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	constexpr auto p1 = "p1"_param(hana::type_c< int >);
	constexpr auto p2 = "p2"_param(types);
	constexpr auto p3 = "p3"_param(types_set);

	static_assert(std::is_same_v< decltype(p1), disposer::param_t<
			decltype("p1"_param), disposer::parameter_parser,
			disposer::parameter< decltype("p1"_param), int >
		> const >);
	static_assert(std::is_same_v< decltype(p2), disposer::param_t<
			decltype("p2"_param), disposer::parameter_parser,
			disposer::parameter< decltype("p2"_param), int, long, float >
		> const >);
	static_assert(std::is_same_v< decltype(p3), disposer::param_t<
			decltype("p3"_param), disposer::parameter_parser,
			disposer::parameter< decltype("p3"_param), int, long, float >
		> const >);

	typename decltype(p1)::type pv1(disposer::parameter_parser(), "5");
	typename decltype(p2)::type pv2(disposer::parameter_parser(), "5");
	typename decltype(p3)::type pv3(disposer::parameter_parser(), "5");

	(void)pv1;
	(void)pv2;
	(void)pv3;
}
