#include <disposer/core/component_init_fn.hpp>

using namespace disposer;


::disposer::disposer d;

template < typename Fn >
void test(Fn const& fn){
	fn(component_init_accessory{
		component_data< void, hana::tuple<> >{hana::make_tuple()}, d, ""});
}


int main(){
	test(component_init_fn([]{ return 0; }));
	test(component_init_fn([](auto){ return 0; }));
	test(component_init_fn([](auto&&){ return 0; }));
	test(component_init_fn([](auto const&){ return 0; }));
	test(component_init_fn([](auto const&&){ return 0; }));

	// Must fail:
// 	test(component_init_fn([](auto&){ return 0; }));

// 	test(component_init_fn([]()mutable{ return 0; }));
// 	test(component_init_fn([](auto)mutable{ return 0; }));
// 	test(component_init_fn([](auto const&)mutable{ return 0; }));
// 	test(component_init_fn([](auto&&)mutable{ return 0; }));
// 	test(component_init_fn([](auto const&&)mutable{ return 0; }));
// 	test(component_init_fn([](auto&)mutable{ return 0; }));
}
