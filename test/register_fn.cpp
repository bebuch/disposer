#include <disposer/core/register_fn.hpp>

#include <boost/hana/tuple.hpp>


using namespace disposer;


struct result{
	using hana_tag = module_register_fn_tag;
};

::disposer::disposer d;

template < typename Fn >
void test(Fn const& fn){
	int state = 0;
	auto const params = hana::make_tuple();
	component_accessory accessory{hana::make_tuple(), state, params, d, ""};

	fn(accessory);
}


int main(){
	test(register_fn([](auto){ return result(); }));
	test(register_fn([](auto&){ return result(); }));
	test(register_fn([](auto&&){ return result(); }));
	test(register_fn([](auto const&){ return result(); }));

	// Must fail:
// 	test(register_fn([](auto const&&){ return result(); }));
// 	test(register_fn([](auto)mutable{ return result(); }));
// 	test(register_fn([](auto const&)mutable{ return result(); }));
// 	test(register_fn([](auto&&)mutable{ return result(); }));
// 	test(register_fn([](auto const&&)mutable{ return result(); }));
// 	test(register_fn([](auto&)mutable{ return result(); }));
}
