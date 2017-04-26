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

void is_disabled_impl(
	std::string_view name,
	std::string_view message,
	std::string_view expected
){

	if(message == expected){
		std::cout << " \033[0;32msuccess:\033[0m " << name << " is disabled\n";
	}else{
		std::cout << " \033[0;31mfail:\033[0m disabled message '"
			<< message << "' != '" << expected << "'\n";
	}
}

template < typename Param >
void is_disabled(Param const& param){
	try{
		param();
	}catch(std::logic_error const& e){
		is_disabled_impl(param.name, e.what(), io_tools::make_string(
			"access parameter '", param.name, "' with disabled type [",
			disposer::type_name<
				typename decltype(+Param::types[hana::int_c< 0 >])::type >(),
			"]"
		));
		return;
	}
	std::cout << " \033[0;31mfail, not disabled:\033[0m " << param.name << '\n';
}

template < typename Param, typename Type >
void is_disabled(Param const& param, Type const& type){
	try{
		param(type);
	}catch(std::logic_error const& e){
		is_disabled_impl(param.name, e.what(), io_tools::make_string(
			"access parameter '", param.name, "' with disabled type [",
			disposer::type_name< typename Type::type >(), "]"
		));
		return;
	}
	std::cout << " \033[0;31mfail, not disabled:\033[0m " << param.name << '\n';
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
	constexpr auto p7 = "p7"_param(hana::type_c< int >,
		[](auto){ return false; });
	constexpr auto p8 = "p8"_param(types,
		[](auto t){ return t == hana::type_c< int >; });

	static_assert(std::is_same_v< decltype(p1), disposer::parameter_maker<
			decltype("p1"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p1"_param), int >
		> const >);
	static_assert(std::is_same_v< decltype(p2), disposer::parameter_maker<
			decltype("p2"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p2"_param), int, long, float >
		> const >);
	static_assert(std::is_same_v< decltype(p3), disposer::parameter_maker<
			decltype("p3"_param),
			disposer::enable_all, disposer::parameter_parser,
			disposer::parameter< decltype("p3"_param), int, long, float >
		> const >);
	static_assert(std::is_same_v< decltype(p4), disposer::parameter_maker<
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
	typename decltype(p7)::type pv7(
		std::move(p7.enabler), std::move(p7.parser), "5");
	typename decltype(p8)::type pv8(
		std::move(p8.enabler), std::move(p8.parser), "5");

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
	is_disabled(pv7);
	is_equal(pv8, hana::type_c< int >, 5);
	is_disabled(pv8, hana::type_c< long >);
	is_disabled(pv8, hana::type_c< float >);

}
