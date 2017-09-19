#include <disposer/core/make_module.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace hana::literals;
using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

constexpr auto list = dimension_list{
		dimension_c< double, char, float >,
		dimension_c< int, bool >,
		dimension_c< short, unsigned, long, long long >
	};


template < std::size_t D >
using ic = index_component< D >;


struct exec{
	constexpr void operator()()const{}
};

constexpr module_init_fn state_dummy{};
constexpr exec_fn exec_dummy{exec{}};


template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(test_1){
	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			set_dimension_fn([](auto const&){
				return solved_dimensions{index_component< 0 >{2}};
			}),
			make("i1"_in, type_ref_c< 0 >, not_required)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			hana::tuple<
				input< decltype("i1"_in), float, false >
			>,
			hana::tuple<>,
			hana::tuple<>,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}

// BOOST_AUTO_TEST_CASE(test_2){
// 	module_make_data module_data{};
// 	auto const module_base_ptr = make_module_ptr(list,
// 		module_configure{
// 			make("i1"_in, type_ref_c< 0 >, required)
// 		}, module_data, state_dummy, exec_dummy);
// 	auto const module_ptr = dynamic_cast< module<
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
