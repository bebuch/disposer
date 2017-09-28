#include <disposer/core/register_module.hpp>

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

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	{
		auto fn = module_register_fn(
				module_configure{}, state_dummy, exec_dummy);
		fn("register_0", disposer.module_declarant());
	}
}

BOOST_AUTO_TEST_CASE(register_1){
	::disposer::disposer disposer;

	constexpr auto list = dimension_list{
			dimension_c< double, char, float >
		};

	struct exec{
		constexpr void operator()()const{}
	};

	constexpr module_init_fn state_dummy{};
	constexpr exec_fn exec_dummy{exec{}};

	auto const register_fn = [=](std::size_t i){
			return module_register_fn(list,
				module_configure{
					set_dimension_fn([i](auto const&){
						return solved_dimensions{index_component< 0 >{i}};
					})
				}, state_dummy, exec_dummy);
		};

	{
		auto fn = register_fn(0);
		fn("register_1_1", disposer.module_declarant());
	}
	{
		auto fn = register_fn(1);
		fn("register_1_2", disposer.module_declarant());
	}
	{
		auto fn = register_fn(2);
		fn("register_1_3", disposer.module_declarant());
	}
}
