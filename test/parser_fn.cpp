#include <disposer/core/parser_fn.hpp>
#include <disposer/core/accessory.hpp>

#include <string_view>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;
using namespace std::literals::string_view_literals;


struct accessory: add_log< accessory >{
	accessory()noexcept{}

	void log_prefix(log_key&&, logsys::stdlogb&)const{}
};


BOOST_AUTO_TEST_CASE(test_1){
	auto const f = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >,
			auto const
		){ return 0; });

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 0));
}

BOOST_AUTO_TEST_CASE(test_2){
	auto const f = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >
		){ return 0; });

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 0));
}

BOOST_AUTO_TEST_CASE(test_3){
	auto const f = parser_fn([](
			std::string_view
		){ return 0; });

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 0));
}


struct fn3{
	template < typename Accessory >
	std::size_t operator()(
		std::string_view,
		hana::basic_type< std::size_t >,
		Accessory
	)const{
		return 3;
	}
};

struct fn2{
	std::size_t operator()(
		std::string_view,
		hana::basic_type< std::size_t >
	)const{
		return 2;
	}
};


struct fn1{
	std::size_t operator()(
		std::string_view
	)const{
		return 1;
	}
};


BOOST_AUTO_TEST_CASE(test_4){
	auto const f = parser_fn(fn3{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 3));
}

BOOST_AUTO_TEST_CASE(test_5){
	auto const f = parser_fn(fn2{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 2));
}

BOOST_AUTO_TEST_CASE(test_6){
	auto const f = parser_fn(fn1{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 1));
}

BOOST_AUTO_TEST_CASE(test_7){
	struct fn: fn1, fn2, fn3{};
	auto const f = parser_fn(fn{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 3));
}

BOOST_AUTO_TEST_CASE(test_8){
	struct fn: fn1, fn2{};
	auto const f = parser_fn(fn{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 2));
}

BOOST_AUTO_TEST_CASE(test_9){
	struct fn: fn1, fn3{};
	auto const f = parser_fn(fn{});

	BOOST_TEST((f(""sv, ""sv, hana::type_c< std::size_t >, accessory{}) == 3));
}

