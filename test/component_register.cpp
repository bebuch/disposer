#include <disposer/core/generate_component.hpp>
#include <disposer/core/generate_module.hpp>

#define BOOST_TEST_MODULE disposer component_register
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

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	{
		auto fn = generate_component("description", init, component_modules{});
		fn("register_0", dir.declarant());
	}
}

BOOST_AUTO_TEST_CASE(register_1){
	directory dir;

	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	auto const generate_fn = [=](std::size_t i){
			return generate_component("description", list,
				component_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, init, component_modules{});
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

BOOST_AUTO_TEST_CASE(register_0_module_1){
	directory dir;

	struct exec{
		constexpr void operator()()const{}
	};

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	{
		auto fn = generate_component("description", init, component_modules{
				make("m1"_module,
					generate_module("description", exec_fn{exec{}}))
			});
		fn("register_0_module_1", dir.declarant());
	}
}
