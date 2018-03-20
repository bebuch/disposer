#include <disposer/core/generate_module.hpp>
#include <disposer/core/directory.hpp>

#define BOOST_TEST_MODULE disposer module_register
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

template < std::size_t D >
using ic = index_component< D >;

template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(register_0){
	directory dir;

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	{
		auto fn = generate_module("description",
			module_configure{}, state_dummy, exec_dummy);
		fn("register_0", dir.declarant());
	}
}

BOOST_AUTO_TEST_CASE(register_1){
	directory dir;

	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	auto const generate_fn = [=](std::size_t i){
			return generate_module("description", list,
				module_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, state_dummy, exec_dummy);
		};

	{
		auto fn = generate_fn(0);
		fn("register_1_1", dir.declarant());
	}
	{
		auto fn = generate_fn(1);
		fn("register_1_2", dir.declarant());
	}
	{
		auto fn = generate_fn(2);
		fn("register_1_3", dir.declarant());
	}
}
