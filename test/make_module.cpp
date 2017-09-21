#include <disposer/core/make_module.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace hana::literals;
using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

template < std::size_t D >
using ic = index_component< D >;


BOOST_AUTO_TEST_CASE(test_1){
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
		BOOST_CHECK_THROW(make_module(3), std::out_of_range);
	}
}

BOOST_AUTO_TEST_CASE(test_2){
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
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{
					index_component< 0 >{2},
					index_component< 1 >{1}
				};
			})
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float, bool >,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(test_3){
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
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{
					index_component< 0 >{2},
					index_component< 1 >{1},
					index_component< 2 >{3}
				};
			})
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float, bool, long long >,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(test_4){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{index_component< 0 >{2}};
			}),
			make("o1"_out, type_ref_c< 0 >)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float >,
			hana::tuple<>,
			hana::tuple< output< decltype("o1"_out), float > >,
			hana::tuple<>,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

BOOST_AUTO_TEST_CASE(test_5){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{index_component< 0 >{2}};
			}),
			make("p1"_param, type_ref_c< 0 >, default_value(3.14159f))
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			type_list< float >,
			hana::tuple<>,
			hana::tuple<>,
			hana::tuple< parameter< decltype("p1"_param), float > >,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}


BOOST_AUTO_TEST_CASE(test_6){
	constexpr auto list = dimension_list{dimension_c< double, char, float >};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
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
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
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
