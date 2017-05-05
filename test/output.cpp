#include <disposer/output.hpp>


int success(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;32msuccess\033[0m\n";
	return 0;
}

int fail(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;31mfail\033[0m;\n";
	return 1;
}


int check(
	std::size_t i,
	std::set< boost::typeindex::ctti_type_index > const& active_types,
	std::set< boost::typeindex::ctti_type_index > const& expected
){
	if(active_types == expected){
		return success(i);
	}else{
		return fail(i);
	}
}


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using ident =
		decltype(hana::typeid_(hana::template_< disposer::self_t >))::type;

	static constexpr auto iops = hana::make_tuple();
	static constexpr auto get_object =
		disposer::iop_list< hana::tuple<> >(iops);

	std::size_t error_count = 0;

	try{
		{
			auto const maker = "v"_out(hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					decltype("v"_out),
					disposer::output< decltype("v"_out), ident, int >,
					disposer::enable_all
				> const >);

			auto object = maker(get_object);

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out), ident, int > >);

			object.put(0);

			error_count = check(0,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ boost::typeindex::ctti_type_index::type_id< int >() }
				});
		}

		{
			auto const maker = "v"_out(types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					decltype("v"_out),
					disposer::output< decltype("v"_out), ident, int, float >,
					disposer::enable_all
				> const >);

			auto object = maker(get_object);

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out), ident, int, float > >);

			object.put(3);
			object.put(3.f);

			error_count = check(1,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ boost::typeindex::ctti_type_index::type_id< int >() },
					{ boost::typeindex::ctti_type_index::type_id< float >() }
				});
		}

		{
			auto const maker = "v"_out(types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					decltype("v"_out),
					disposer::output< decltype("v"_out), ident, int, float >,
					disposer::enable_all
				> const >);

			auto object = maker(get_object);

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out), ident, int, float > >);

			object.put(3);
			object.put(3.f);

			error_count = check(2,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ boost::typeindex::ctti_type_index::type_id< int >() },
					{ boost::typeindex::ctti_type_index::type_id< float >() }
				});
		}

		if(error_count == 0){
			std::cout << "\033[0;32mSUCCESS\033[0m\n";
		}else{
			std::cout << "\033[0;31mFAILS:\033[0m " << error_count << '\n';
		}
	}catch(std::exception const& e){
		std::cerr << "Unexpected exception: " << e.what() << '\n';
	}catch(...){
		std::cerr << "Unexpected unknown exception\n";
	}

	return error_count;
}
