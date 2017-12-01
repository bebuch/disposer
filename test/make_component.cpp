#include <disposer/core/make_component.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

template < std::size_t D >
using ic = index_component< D >;

template < typename ... T > struct morph{};

::disposer::disposer d;


BOOST_AUTO_TEST_CASE(set_dimension_fn_1){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	component_make_data component_data{};
	auto const make_component = [=](std::size_t i){
			return make_component_ptr(list,
				component_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, component_modules{}, d, component_data, init);
		};

	{
		auto component_base_ptr = make_component(0);
		auto const component_ptr = dynamic_cast< component<
				type_list< double >,
				hana::tuple<>,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1);
		auto const component_ptr = dynamic_cast< component<
				type_list< char >,
				hana::tuple<>,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2);
		auto const component_ptr = dynamic_cast< component<
				type_list< float >,
				hana::tuple<>,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3), std::out_of_range,
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

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	component_make_data component_data{};
	auto const make_component = [=](std::size_t i1, std::size_t i2){
			return make_component_ptr(list,
				component_configure{
					set_dimension_fn([i1, i2](auto const&){
						return solved_dimensions{
							index_component< 0 >{i1},
							index_component< 1 >{i2}
						};
					})
				}, component_modules{}, d, component_data, init);
		};

	{
		auto component_base_ptr = make_component(0, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, int >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, int >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, int >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, bool >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, bool >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, bool >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2), std::out_of_range,
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

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	component_make_data component_data{};
	auto const make_component =
		[=](std::size_t i1, std::size_t i2, std::size_t i3){
			return make_component_ptr(list,
				component_configure{
					set_dimension_fn([i1, i2, i3](auto const&){
						return solved_dimensions{
							index_component< 0 >{i1},
							index_component< 1 >{i2},
							index_component< 2 >{i3}
						};
					})
				}, component_modules{}, d, component_data, init);
		};

	{
		auto component_base_ptr = make_component(0, 0, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, int, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 0, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, int, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 0, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, int, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 1, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, bool, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 1, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, bool, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 1, 0);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, bool, short >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 0, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, int, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 0, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, int, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 0, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, int, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 1, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, bool, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 1, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, bool, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 1, 1);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, bool, unsigned >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 0, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, int, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 0, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, int, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 0, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, int, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 1, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, bool, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 1, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, bool, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 1, 2);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, bool, long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 0, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, int, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 0, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, int, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 0, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, int, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		auto component_base_ptr = make_component(0, 1, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< double, bool, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1, 1, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< char, bool, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2, 1, 3);
		auto const component_ptr = dynamic_cast< component<
				type_list< float, bool, long long >,
				hana::tuple<>, init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(0, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(0, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2, 0), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 0 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2, 1), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 1 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2, 2), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 2 is valid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2, 3), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 3 is valid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 0, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 0 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(3, 1, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 1 is valid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(0, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 0 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(1, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 1 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
	{
		BOOST_CHECK_EXCEPTION(make_component(2, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 2 is valid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}

	{
		BOOST_CHECK_EXCEPTION(make_component(3, 2, 4), std::out_of_range,
			([](std::out_of_range const& e){
				return e.what() == "index is out of range: "
					"dimension number 0 has 3 types (index 3 is invalid), "
					"dimension number 1 has 2 types (index 2 is invalid), "
					"dimension number 2 has 4 types (index 4 is invalid)"sv;
			}));
	}
}

BOOST_AUTO_TEST_CASE(param_0){
	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	component_make_data component_data{};
	auto const component_base_ptr = make_component_ptr(dimension_list{},
		component_configure{
			make("p1"_param, free_type_c< float >, "description",
				default_value(3.14159f))
		}, component_modules{}, d, component_data, init);
	auto const component_ptr = dynamic_cast< component<
			type_list<>,
			hana::tuple< parameter< decltype("p1"_param), float > >,
			init_fn
		>* >(component_base_ptr.get());
	BOOST_TEST(component_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(param_1){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	component_make_data component_data{};
	auto const make_component = [=](std::size_t i1){
			return make_component_ptr(list,
				component_configure{
					set_dimension_fn([i1](auto const&){
						return solved_dimensions{index_component< 0 >{i1}};
					}),
					make("p1"_param, type_ref_c< 0 >, "description",
						default_value())
				}, component_modules{}, d, component_data, init);
		};

	{
		auto component_base_ptr = make_component(0);
		auto const component_ptr = dynamic_cast< component<
				type_list< double >,
				hana::tuple< parameter< decltype("p1"_param), double > >,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(1);
		auto const component_ptr = dynamic_cast< component<
				type_list< char >,
				hana::tuple< parameter< decltype("p1"_param), char > >,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
	{
		auto component_base_ptr = make_component(2);
		auto const component_ptr = dynamic_cast< component<
				type_list< float >,
				hana::tuple< parameter< decltype("p1"_param), float > >,
				init_fn
			>* >(component_base_ptr.get());
		BOOST_TEST(component_ptr != nullptr);
	}
}


struct component_2_component_init_fn{
	template < typename Accessory >
	constexpr auto operator()(Accessory const& accessory)const{
		decltype(auto) p1 = accessory("p1"_param);
		static_assert(
			std::is_same_v< decltype(p1), double const& > ||
			std::is_same_v< decltype(p1), char const& > ||
			std::is_same_v< decltype(p1), float const& >);

		auto dim1 = accessory.dimension(hana::size_c< 0 >);
		static_assert(
			dim1 == hana::type_c< double > ||
			dim1 == hana::type_c< char > ||
			dim1 == hana::type_c< float >);

		auto dim2 = accessory.dimension(hana::size_c< 1 >);
		static_assert(
			dim2 == hana::type_c< int > ||
			dim2 == hana::type_c< bool >);

		return 0;
	}
};

BOOST_AUTO_TEST_CASE(component_2){
	constexpr auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >
		};
	constexpr component_init_fn init{component_2_component_init_fn{}};

	component_make_data component_data{};
	auto const component_base_ptr = make_component_ptr(list,
		component_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{ic< 0 >{2}};
			}),
			make("p1"_param, type_ref_c< 0 >, "description", default_value()),
			set_dimension_fn([](auto const& accessory){
				decltype(auto) p1 = accessory("p1"_param);
				static_assert(
					std::is_same_v< decltype(p1), double const& > ||
					std::is_same_v< decltype(p1), char const& > ||
					std::is_same_v< decltype(p1), float const& >);

				auto dim1 = accessory.dimension(hana::size_c< 0 >);
				static_assert(
					dim1 == hana::type_c< double > ||
					dim1 == hana::type_c< char > ||
					dim1 == hana::type_c< float >);

				return solved_dimensions{ic< 1 >{1}};
			}),
		}, component_modules{}, d, component_data, init);
	auto const component_ptr = dynamic_cast< component<
			type_list< float, bool >,
			hana::tuple< parameter< decltype("p1"_param), float > >,
			component_2_component_init_fn
		>* >(component_base_ptr.get());
	BOOST_TEST(component_ptr != nullptr);
}
