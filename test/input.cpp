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

BOOST_AUTO_TEST_CASE(test_2){
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

BOOST_AUTO_TEST_CASE(test_3){
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

// BOOST_AUTO_TEST_CASE(test_2){
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
