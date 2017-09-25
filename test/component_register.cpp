#include <disposer/core/register_component.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

template < std::size_t D >
using ic = index_component< D >;

template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(register_0){
	::disposer::disposer disposer;

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	{
		auto fn = component_register_fn(component_modules{}, init);
		fn("register_0", disposer.component_declarant());
	}
}

BOOST_AUTO_TEST_CASE(register_1){
	::disposer::disposer disposer;

	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct init_fn{
		constexpr auto operator()()const{ return 0; }
	};

	constexpr component_init_fn init{init_fn{}};

	auto const register_fn = [=](std::size_t i){
			return component_register_fn(list,
				component_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, component_modules{}, init);
		};

	{
		auto fn = register_fn(0);
		fn("register_1_1", disposer.component_declarant());
	}
	{
		auto fn = register_fn(1);
		fn("register_1_2", disposer.component_declarant());
	}
	{
		auto fn = register_fn(2);
		fn("register_1_3", disposer.component_declarant());
	}
}
