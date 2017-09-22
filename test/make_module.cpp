#include <disposer/core/make_module.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

template < std::size_t D >
using ic = index_component< D >;

template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(set_dimension_fn_1){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module = [=](std::size_t i){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double >,
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<>,
				void,
				exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char >,
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<>,
				void,
				exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float >,
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<>,
				void,
				exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid)"sv;
			}));
	}
}

BOOST_AUTO_TEST_CASE(set_dimension_fn_2){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module = [=](std::size_t i1, std::size_t i2){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1, i2](auto const&){
						return solved_dimensions{
							index_component< 0 >{i1},
							index_component< 1 >{i2}
						};
					})
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}
}


BOOST_AUTO_TEST_CASE(set_dimension_fn_3){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >,
			dimension_c< short, unsigned, long, long long >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module =
		[=](std::size_t i1, std::size_t i2, std::size_t i3){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1, i2, i3](auto const&){
						return solved_dimensions{
							index_component< 0 >{i1},
							index_component< 1 >{i2},
							index_component< 2 >{i3}
						};
					})
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0, 0, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 1, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool, short >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 0, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 1, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool, unsigned >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 0, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 1, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1, 2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool, long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 0, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		auto module_base_ptr = make_module(0, 1, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1, 3);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool, long long >,
				hana::tuple<>, hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(0, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(0, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(3, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(0, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(1, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_module(2, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_module(3, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
}

BOOST_AUTO_TEST_CASE(output_0){
	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(dimension_list{},
		module_configure{
			make("o1"_out, free_type_c< char >)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list<>, hana::tuple<>,
			hana::tuple< output< decltype("o1"_out), char > >,
			hana::tuple<>, void, exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(output_1){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module =
		[=](std::size_t i1){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1](auto const&){
						return solved_dimensions{index_component< 0 >{i1}};
					}),
					make("o1"_out, type_ref_c< 0 >)
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double >, hana::tuple<>,
				hana::tuple< output< decltype("o1"_out), double > >,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char >, hana::tuple<>,
				hana::tuple< output< decltype("o1"_out), char > >,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float >, hana::tuple<>,
				hana::tuple< output< decltype("o1"_out), float > >,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
}

BOOST_AUTO_TEST_CASE(output_2){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module =
		[=](std::size_t i1, std::size_t i2){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1, i2](auto const&){
						return solved_dimensions{
							index_component< 0 >{i1},
							index_component< 1 >{i2}
						};
					}),
					make("o1"_out, wrapped_type_ref_c< morph, 0, 1 >)
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, int >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< double, int > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, int >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< char, int > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 0);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, int >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< float, int > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(0, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< double, bool >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< double, bool > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char, bool >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< char, bool > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2, 1);
		auto const module_ptr = dynamic_cast< module<
				type_list< float, bool >, hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), morph< float, bool > >
				>,
				hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
}

BOOST_AUTO_TEST_CASE(param_0){
	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(dimension_list{},
		module_configure{
			make("p1"_param, free_type_c< float >, default_value(3.14159f))
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list<>, hana::tuple<>, hana::tuple<>,
			hana::tuple< parameter< decltype("p1"_param), float > >,
			void, exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(param_1){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module = [=](std::size_t i1){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1](auto const&){
						return solved_dimensions{index_component< 0 >{i1}};
					}),
					make("p1"_param, type_ref_c< 0 >, default_value())
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double >, hana::tuple<>, hana::tuple<>,
				hana::tuple< parameter< decltype("p1"_param), double > >,
				void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char >, hana::tuple<>, hana::tuple<>,
				hana::tuple< parameter< decltype("p1"_param), char > >,
				void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float >, hana::tuple<>, hana::tuple<>,
				hana::tuple< parameter< decltype("p1"_param), float > >,
				void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
}


BOOST_AUTO_TEST_CASE(input_0){
	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(dimension_list{},
		module_configure{
			make("i1"_in, free_type_c< float >, not_required)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list<>,
			hana::tuple< input< decltype("i1"_in), float, false > >,
			hana::tuple<>, hana::tuple<>, void, exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(input_1){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const make_module = [=](std::size_t i1){
			return make_module_ptr(list,
				module_configure{
					set_dimension_fn([i1](auto const&){
						return solved_dimensions{index_component< 0 >{i1}};
					}),
					make("i1"_in, type_ref_c< 0 >, not_required)
				}, module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(0);
		auto const module_ptr = dynamic_cast< module<
				type_list< double >,
				hana::tuple< input< decltype("i1"_in), double, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(1);
		auto const module_ptr = dynamic_cast< module<
				type_list< char >,
				hana::tuple< input< decltype("i1"_in), char, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(2);
		auto const module_ptr = dynamic_cast< module<
				type_list< float >,
				hana::tuple< input< decltype("i1"_in), float, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
}

BOOST_AUTO_TEST_CASE(input_auto_1){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	auto const make_module = [=](auto type){
			output< decltype("o1"_out), typename decltype(type)::type > o1{0};
			module_make_data module_data{{}, {}, {}, {{"i1", &o1}}, {}, {}};
			return make_module_ptr(list,
				module_configure{make("i1"_in, type_ref_c< 0 >)},
				module_data, state_dummy, exec_dummy);
		};

	{
		auto module_base_ptr = make_module(hana::type_c< double >);
		auto const module_ptr = dynamic_cast< module<
				type_list< double >,
				hana::tuple< input< decltype("i1"_in), double, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(hana::type_c< char >);
		auto const module_ptr = dynamic_cast< module<
				type_list< char >,
				hana::tuple< input< decltype("i1"_in), char, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
	{
		auto module_base_ptr = make_module(hana::type_c< float >);
		auto const module_ptr = dynamic_cast< module<
				type_list< float >,
				hana::tuple< input< decltype("i1"_in), float, false > >,
				hana::tuple<>, hana::tuple<>, void, exec
			>* >(module_base_ptr.get());
		BOOST_TEST(module_ptr != nullptr);
	}
}


struct test_2_module_init_fn{
	template < typename Accessory >
	constexpr auto operator()(Accessory const& accessory)const{
		auto i1 = accessory("i1"_in);
		static_assert(std::is_same_v< decltype(i1), bool >);
		return 0;
	}
};

BOOST_AUTO_TEST_CASE(test_7){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{test_2_module_init_fn{}};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{ic< 0 >{2}};
			}),
			make("i1"_in, type_ref_c< 0 >, not_required)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float >,
			hana::tuple< input< decltype("i1"_in), float, false > >,
			hana::tuple<>,
			hana::tuple<>,
			test_2_module_init_fn,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}


struct test_3_exec{
	template < typename Accessory >
	constexpr void operator()(Accessory& accessory)const{
		auto& i1 = accessory("i1"_in);
		static_assert(
			std::is_same_v< decltype(i1),
				exec_input< decltype("i1"_in), double, false >& > ||
			std::is_same_v< decltype(i1),
				exec_input< decltype("i1"_in), char, false >& > ||
			std::is_same_v< decltype(i1),
				exec_input< decltype("i1"_in), float, false >& >);
	}
};

BOOST_AUTO_TEST_CASE(test_8){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{test_3_exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{ic< 0 >{2}};
			}),
			make("i1"_in, type_ref_c< 0 >, not_required)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float >,
			hana::tuple< input< decltype("i1"_in), float, false > >,
			hana::tuple<>,
			hana::tuple<>,
			void,
			test_3_exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

// 	constexpr auto list = dimension_list{
// 			dimension_c< double, char, float >,
// 			dimension_c< int, bool >,
// 			dimension_c< short, unsigned, long, long long >
// 		};

// BOOST_AUTO_TEST_CASE(test_9){
// 	module_make_data module_data{};
// 	auto const module_base_ptr = make_module_ptr(list,
// 		module_configure{
// 			make("i1"_in, type_ref_c< 0 >, required)
// 		}, module_data, state_dummy, exec_dummy);
// 	auto const module_ptr = dynamic_cast< module<
// 			type_list< float >,
// 			hana::tuple<
// 				input< decltype("i1"_in), float, false >
// 			>,
// 			hana::tuple<>,
// 			hana::tuple<>,
// 			void,
// 			exec
// 		>* >(module_base_ptr.get());
// 	BOOST_TEST(module_ptr != nullptr);
// }
