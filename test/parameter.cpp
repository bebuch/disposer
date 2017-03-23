#include <disposer/parameter.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, long, float >;
constexpr auto types_set = hana::to_set(types);


struct string_parser{
	std::string operator()(
		std::string const& value,
		hana::basic_type< std::string >
	)const{
		return value;
	}
};


template < typename Value >
void is_equal_impl(std::string_view name, Value const& param, Value const& value){
	if(param == value){
		std::cout << " \033[0;32msuccess:\033[0m ";
	}else{
		std::cout << " \033[0;31mfail:\033[0m ";
	}
	std::cout << name << " = " << value << "\n";
}

template < typename Param, typename Value >
void is_equal(Param const& param, Value const& value){
	is_equal_impl(param.name, param(), value);
}

template < typename Param, typename Type, typename Value >
void is_equal(Param const& param, Type const& type, Value const& value){
	is_equal_impl(param.name, param(type), value);
}


int main(){
	constexpr auto p1 = "p1"_param(hana::type_c< int >);
	constexpr auto p2 = "p2"_param(types);
	constexpr auto p3 = "p3"_param(types_set);
	constexpr auto p4 = "p4"_param(hana::type_c< std::string >,
		disposer::enable_all(), string_parser());
	constexpr auto p5 = "p5"_param(hana::type_c< int >,
		disposer::enable_all(),
		[](std::string const&, hana::basic_type< int >){ return 3; });
	constexpr auto p6 = "p6"_param(types,
		disposer::enable_all(),
		[](std::string const&, auto t)
			->typename decltype(t)::type{ return {}; });

	static_assert(std::is_same_v< decltype(p1), disposer::param_t<
			decltype("p1"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p1"_param), int >
		> const >);
	static_assert(std::is_same_v< decltype(p2), disposer::param_t<
			decltype("p2"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p2"_param), int, long, float >
		> const >);
	static_assert(std::is_same_v< decltype(p3), disposer::param_t<
			decltype("p3"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p3"_param), int, long, float >
		> const >);
	static_assert(std::is_same_v< decltype(p4), disposer::param_t<
			decltype("p4"_param),
			disposer::enable_all, string_parser,
			disposer::parameter< decltype("p4"_param), std::string >
		> const >);

	typename decltype(p1)::type pv1(
		std::move(p1.enabler), std::move(p1.parser), "5");
	typename decltype(p2)::type pv2(
		std::move(p2.enabler), std::move(p2.parser), "5");
	typename decltype(p3)::type pv3(
		std::move(p3.enabler), std::move(p3.parser), "5");
	typename decltype(p4)::type pv4(
		std::move(p4.enabler), std::move(p4.parser), "5");
	typename decltype(p5)::type pv5(
		std::move(p5.enabler), std::move(p5.parser), "5");
	typename decltype(p6)::type pv6(
		std::move(p6.enabler), std::move(p6.parser), "5");

	is_equal(pv1, 5);
	is_equal(pv2, hana::type_c< int >, 5);
	is_equal(pv2, hana::type_c< long >, 5l);
	is_equal(pv2, hana::type_c< float >, 5.f);
	is_equal(pv3, hana::type_c< int >, 5);
	is_equal(pv3, hana::type_c< long >, 5l);
	is_equal(pv3, hana::type_c< float >, 5.f);
	is_equal(pv4, std::string("5"));
	is_equal(pv5, 3);
	is_equal(pv6, hana::type_c< int >, 0);
	is_equal(pv6, hana::type_c< long >, 0l);
	is_equal(pv6, hana::type_c< float >, 0.f);
}
