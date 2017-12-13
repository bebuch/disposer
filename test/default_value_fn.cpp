#include <disposer/core/default_value_fn.hpp>
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


constexpr auto type = hana::type_c< std::size_t >;


template < typename ... > struct morph{morph(std::size_t){}};
template < typename T > using self = T;


constexpr dimension_referrer< self, 0 > ref_self;
constexpr dimension_referrer< morph, 0 > ref_morph;

constexpr dimension_list< dimension< std::size_t > > list1;


BOOST_AUTO_TEST_CASE(test_1_user_help_0){
	auto const f1 = default_value_fn([](
			hana::basic_type< std::size_t >,
			auto const
		){ return 0; }, []{ return "0"; });
	auto const f2 = default_value_fn([](
			hana::basic_type< std::size_t >,
			auto const
		)noexcept{ return 0; }, []{ return "0"; });

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 0));
	BOOST_TEST((f2(""sv, type, accessory{}) == 0));

	BOOST_TEST((f1.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");
	BOOST_TEST((f2.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");
}

BOOST_AUTO_TEST_CASE(test_2){
	auto const f1 = default_value_fn([](
			hana::basic_type< std::size_t >
		){ return 0; });
	auto const f2 = default_value_fn([](
			hana::basic_type< std::size_t >
		)noexcept{ return 0; });

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 0));
	BOOST_TEST((f2(""sv, type, accessory{}) == 0));

	BOOST_TEST((f1.help_text_fn(ref_self, list1)) == "default value: 0");
	BOOST_TEST((f2.help_text_fn(ref_self, list1)) == "default value: 0");
}

BOOST_AUTO_TEST_CASE(test_2_user_help_0){
	auto const f1 = default_value_fn([](
			hana::basic_type< std::size_t >
		){ return 0; }, []{ return "0"; });
	auto const f2 = default_value_fn([](
			hana::basic_type< std::size_t >
		)noexcept{ return 0; }, []{ return "0"; });

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 0));
	BOOST_TEST((f2(""sv, type, accessory{}) == 0));

	BOOST_TEST((f1.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");
	BOOST_TEST((f2.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");
}

BOOST_AUTO_TEST_CASE(test_3){
	auto const f1 = default_value_fn([](){ return 0; });
	auto const f2 = default_value_fn([]()noexcept{ return 0; });

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 0));
	BOOST_TEST((f2(""sv, type, accessory{}) == 0));

	BOOST_TEST((f1.help_text_fn(ref_self, list1)) == "default value: 0");
	BOOST_TEST((f2.help_text_fn(ref_self, list1)) == "default value: 0");

	// TODO: print not supported
	BOOST_TEST((f1.help_text_fn(ref_morph, list1)) ==
		"default value: (no output support)");
	BOOST_TEST((f2.help_text_fn(ref_morph, list1)) ==
		"default value: (no output support)");
}

BOOST_AUTO_TEST_CASE(test_3_user_help_0){
	auto const f1 = default_value_fn([](){ return 0; },
		[]{ return "0"; });
	auto const f2 = default_value_fn([]()noexcept{ return 0; },
		[]{ return "0"; });

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 0));
	BOOST_TEST((f2(""sv, type, accessory{}) == 0));

	BOOST_TEST((f1.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");
	BOOST_TEST((f2.help_text_fn(ref_self, list1)) ==
		"user defined default value help: 0");

	BOOST_TEST((f1.help_text_fn(ref_morph, list1)) ==
		"user defined default value help: 0");
	BOOST_TEST((f2.help_text_fn(ref_morph, list1)) ==
		"user defined default value help: 0");
}


struct fn3{
	template < typename Accessory >
	std::size_t operator()(
		hana::basic_type< std::size_t >,
		Accessory
	)const{
		return 3;
	}
};

struct fn3_nothrow{
	template < typename Accessory >
	std::size_t operator()(
		hana::basic_type< std::size_t >,
		Accessory
	)const noexcept{
		return 3;
	}
};

struct fn2{
	std::size_t operator()(
		hana::basic_type< std::size_t >
	)const{
		return 2;
	}
};

struct fn2_nothrow{
	std::size_t operator()(
		hana::basic_type< std::size_t >
	)const noexcept{
		return 2;
	}
};


struct fn1{
	std::size_t operator()()const{
		return 1;
	}
};


struct fn1_nothrow{
	std::size_t operator()()const noexcept{
		return 1;
	}
};


BOOST_AUTO_TEST_CASE(test_4){
	auto const f1 = default_value_fn(fn3{});
	auto const f2 = default_value_fn(fn3_nothrow{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 3));
	BOOST_TEST((f2(""sv, type, accessory{}) == 3));
}

BOOST_AUTO_TEST_CASE(test_5){
	auto const f1 = default_value_fn(fn2{});
	auto const f2 = default_value_fn(fn2_nothrow{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 2));
	BOOST_TEST((f2(""sv, type, accessory{}) == 2));
}

BOOST_AUTO_TEST_CASE(test_6){
	auto const f1 = default_value_fn(fn1{});
	auto const f2 = default_value_fn(fn1_nothrow{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 1));
	BOOST_TEST((f2(""sv, type, accessory{}) == 1));
}

BOOST_AUTO_TEST_CASE(test_default){
	struct v1{v1()noexcept(false){}};
	struct v2{v2()noexcept(true){}};

	auto const f1 = default_value();
	auto const f2 = default_value();

	static_assert(!noexcept(f1(""sv, hana::type_c< v1 >, accessory{})));
	static_assert(noexcept(f2(""sv, hana::type_c< v2 >, accessory{})));
}

BOOST_AUTO_TEST_CASE(test_copy){
	struct v1{v1()=default; v1(v1 const&)noexcept(false){}};
	struct v2{v2()=default; v2(v2 const&)noexcept(true){}};

	auto const f1 = default_value(v1{});
	auto const f2 = default_value(v2{});

	static_assert(!noexcept(f1(""sv, hana::type_c< v1 >, accessory{})));
	static_assert(noexcept(f2(""sv, hana::type_c< v2 >, accessory{})));
}

BOOST_AUTO_TEST_CASE(test_7){
	struct fnl1: fn1, fn2, fn3{};
	auto const f1 = default_value_fn(fnl1{});
	struct fnl2: fn1_nothrow, fn2, fn3{};
	auto const f2 = default_value_fn(fnl2{});
	struct fnl3: fn1, fn2_nothrow, fn3{};
	auto const f3 = default_value_fn(fnl3{});
	struct fnl4: fn1_nothrow, fn2_nothrow, fn3{};
	auto const f4 = default_value_fn(fnl4{});
	struct fnl5: fn1, fn2, fn3_nothrow{};
	auto const f5 = default_value_fn(fnl5{});
	struct fnl6: fn1_nothrow, fn2, fn3_nothrow{};
	auto const f6 = default_value_fn(fnl6{});
	struct fnl7: fn1, fn2_nothrow, fn3_nothrow{};
	auto const f7 = default_value_fn(fnl7{});
	struct fnl8: fn1_nothrow, fn2_nothrow, fn3_nothrow{};
	auto const f8 = default_value_fn(fnl8{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));
	static_assert(!noexcept(f3(""sv, type, accessory{})));
	static_assert(noexcept(f4(""sv, type, accessory{})));
	static_assert(!noexcept(f5(""sv, type, accessory{})));
	static_assert(noexcept(f6(""sv, type, accessory{})));
	static_assert(!noexcept(f7(""sv, type, accessory{})));
	static_assert(noexcept(f8(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 1));
	BOOST_TEST((f2(""sv, type, accessory{}) == 1));
	BOOST_TEST((f3(""sv, type, accessory{}) == 1));
	BOOST_TEST((f4(""sv, type, accessory{}) == 1));
	BOOST_TEST((f5(""sv, type, accessory{}) == 1));
	BOOST_TEST((f6(""sv, type, accessory{}) == 1));
	BOOST_TEST((f7(""sv, type, accessory{}) == 1));
	BOOST_TEST((f8(""sv, type, accessory{}) == 1));
}

BOOST_AUTO_TEST_CASE(test_8){
	struct fnl1: fn1, fn2{};
	auto const f1 = default_value_fn(fnl1{});
	struct fnl2: fn1_nothrow, fn2{};
	auto const f2 = default_value_fn(fnl2{});
	struct fnl3: fn1, fn2_nothrow{};
	auto const f3 = default_value_fn(fnl3{});
	struct fnl4: fn1_nothrow, fn2_nothrow{};
	auto const f4 = default_value_fn(fnl4{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));
	static_assert(!noexcept(f3(""sv, type, accessory{})));
	static_assert(noexcept(f4(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 1));
	BOOST_TEST((f2(""sv, type, accessory{}) == 1));
	BOOST_TEST((f3(""sv, type, accessory{}) == 1));
	BOOST_TEST((f4(""sv, type, accessory{}) == 1));
}

BOOST_AUTO_TEST_CASE(test_9){
	struct fnl1: fn2, fn3{};
	auto const f1 = default_value_fn(fnl1{});
	struct fnl2: fn2_nothrow, fn3{};
	auto const f2 = default_value_fn(fnl2{});
	struct fnl3: fn2, fn3_nothrow{};
	auto const f3 = default_value_fn(fnl3{});
	struct fnl4: fn2_nothrow, fn3_nothrow{};
	auto const f4 = default_value_fn(fnl4{});

	static_assert(!noexcept(f1(""sv, type, accessory{})));
	static_assert(noexcept(f2(""sv, type, accessory{})));
	static_assert(!noexcept(f3(""sv, type, accessory{})));
	static_assert(noexcept(f4(""sv, type, accessory{})));

	BOOST_TEST((f1(""sv, type, accessory{}) == 2));
	BOOST_TEST((f2(""sv, type, accessory{}) == 2));
	BOOST_TEST((f3(""sv, type, accessory{}) == 2));
	BOOST_TEST((f4(""sv, type, accessory{}) == 2));
}

