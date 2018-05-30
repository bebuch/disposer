#include <disposer/core/parser_fn.hpp>
#include <disposer/core/ref.hpp>

#include <string_view>

#define BOOST_TEST_MODULE disposer parser_fn
#include <boost/test/included/unit_test.hpp>


using namespace disposer;
using namespace std::literals::string_view_literals;


struct ref: logsys::log_ref{
	ref()noexcept: logsys::log_ref("") {}
};


constexpr auto type = hana::type_c< std::size_t >;


BOOST_AUTO_TEST_CASE(test_1){
	auto const f1 = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >,
			auto const
		){ return 0; });
	auto const f2 = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >,
			auto const
		)noexcept{ return 0; });

	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 0));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 0));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_2){
	auto const f1 = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >
		){ return 0; });
	auto const f2 = parser_fn([](
			std::string_view,
			hana::basic_type< std::size_t >
		)noexcept{ return 0; });

	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 0));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 0));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_3){
	auto const f1 = parser_fn([](
			std::string_view
		){ return 0; });
	auto const f2 = parser_fn([](
			std::string_view
		)noexcept{ return 0; });


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 0));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 0));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}


struct fn3{
	template < typename Ref >
	std::size_t operator()(
		std::string_view,
		hana::basic_type< std::size_t >,
		Ref
	)const{
		return 3;
	}
};

struct fn3_nothrow{
	template < typename Ref >
	std::size_t operator()(
		std::string_view,
		hana::basic_type< std::size_t >,
		Ref
	)const noexcept{
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

struct fn2_nothrow{
	std::size_t operator()(
		std::string_view,
		hana::basic_type< std::size_t >
	)const noexcept{
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


struct fn1_nothrow{
	std::size_t operator()(
		std::string_view
	)const noexcept{
		return 1;
	}
};


BOOST_AUTO_TEST_CASE(test_4){
	auto const f1 = parser_fn(fn3{});
	auto const f2 = parser_fn(fn3_nothrow{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 3));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 3));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_5){
	auto const f1 = parser_fn(fn2{});
	auto const f2 = parser_fn(fn2_nothrow{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 2));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 2));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_6){
	auto const f1 = parser_fn(fn1{});
	auto const f2 = parser_fn(fn1_nothrow{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 1));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_7){
	struct fnl1: fn1, fn2, fn3{
		using fn1::operator();
		using fn2::operator();
		using fn3::operator();
	};
	auto const f1 = parser_fn(fnl1{});
	struct fnl2: fn1_nothrow, fn2, fn3{
		using fn1_nothrow::operator();
		using fn2::operator();
		using fn3::operator();
	};
	auto const f2 = parser_fn(fnl2{});
	struct fnl3: fn1, fn2_nothrow, fn3{
		using fn1::operator();
		using fn2_nothrow::operator();
		using fn3::operator();
	};
	auto const f3 = parser_fn(fnl3{});
	struct fnl4: fn1_nothrow, fn2_nothrow, fn3{
		using fn1_nothrow::operator();
		using fn2_nothrow::operator();
		using fn3::operator();
	};
	auto const f4 = parser_fn(fnl4{});
	struct fnl5: fn1, fn2, fn3_nothrow{
		using fn1::operator();
		using fn2::operator();
		using fn3_nothrow::operator();
	};
	auto const f5 = parser_fn(fnl5{});
	struct fnl6: fn1_nothrow, fn2, fn3_nothrow{
		using fn1_nothrow::operator();
		using fn2::operator();
		using fn3_nothrow::operator();
	};
	auto const f6 = parser_fn(fnl6{});
	struct fnl7: fn1, fn2_nothrow, fn3_nothrow{
		using fn1::operator();
		using fn2_nothrow::operator();
		using fn3_nothrow::operator();
	};
	auto const f7 = parser_fn(fnl7{});
	struct fnl8: fn1_nothrow, fn2_nothrow, fn3_nothrow{
		using fn1_nothrow::operator();
		using fn2_nothrow::operator();
		using fn3_nothrow::operator();
	};
	auto const f8 = parser_fn(fnl8{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f3(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f4(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f5(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f6(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f7(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f8(""sv, ""sv, type, ref{}) == 1));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
	static_assert(!noexcept(f3(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f4(""sv, ""sv, type, ref{})));
	static_assert(!noexcept(f5(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f6(""sv, ""sv, type, ref{})));
	static_assert(!noexcept(f7(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f8(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_8){
	struct fnl1: fn1, fn2{
		using fn1::operator();
		using fn2::operator();
	};
	auto const f1 = parser_fn(fnl1{});
	struct fnl2: fn1_nothrow, fn2{
		using fn1_nothrow::operator();
		using fn2::operator();
	};
	auto const f2 = parser_fn(fnl2{});
	struct fnl3: fn1, fn2_nothrow{
		using fn1::operator();
		using fn2_nothrow::operator();
	};
	auto const f3 = parser_fn(fnl3{});
	struct fnl4: fn1_nothrow, fn2_nothrow{
		using fn1_nothrow::operator();
		using fn2_nothrow::operator();
	};
	auto const f4 = parser_fn(fnl4{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f3(""sv, ""sv, type, ref{}) == 1));
	BOOST_TEST((f4(""sv, ""sv, type, ref{}) == 1));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
	static_assert(!noexcept(f3(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f4(""sv, ""sv, type, ref{})));
}

BOOST_AUTO_TEST_CASE(test_9){
	struct fnl1: fn2, fn3{
		using fn2::operator();
		using fn3::operator();
	};
	auto const f1 = parser_fn(fnl1{});
	struct fnl2: fn2_nothrow, fn3{
		using fn2_nothrow::operator();
		using fn3::operator();
	};
	auto const f2 = parser_fn(fnl2{});
	struct fnl3: fn2, fn3_nothrow{
		using fn2::operator();
		using fn3_nothrow::operator();
	};
	auto const f3 = parser_fn(fnl3{});
	struct fnl4: fn2_nothrow, fn3_nothrow{
		using fn2_nothrow::operator();
		using fn3_nothrow::operator();
	};
	auto const f4 = parser_fn(fnl4{});


	BOOST_TEST((f1(""sv, ""sv, type, ref{}) == 2));
	BOOST_TEST((f2(""sv, ""sv, type, ref{}) == 2));
	BOOST_TEST((f3(""sv, ""sv, type, ref{}) == 2));
	BOOST_TEST((f4(""sv, ""sv, type, ref{}) == 2));

	static_assert(!noexcept(f1(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f2(""sv, ""sv, type, ref{})));
	static_assert(!noexcept(f3(""sv, ""sv, type, ref{})));
	static_assert(noexcept(f4(""sv, ""sv, type, ref{})));
}

