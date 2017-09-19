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

constexpr state_maker_fn state_dummy{};
constexpr exec_fn exec_dummy{exec{}};


template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(test_1){
	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{
			module_init_fn([](auto const&){
				return solved_dimensions{index_component< 0 >{2}};
			}),
			make("o1"_out, type_ref_c< 0 >)
		}, module_data, state_dummy, exec_dummy);
	auto const module_ptr = dynamic_cast< module<
			hana::tuple<>,
			hana::tuple<
				output< decltype("o1"_out), float >
			>,
			hana::tuple<>,
			void,
			exec
		>* >(module_base_ptr.get());
	BOOST_TEST(module_ptr != nullptr);
}
