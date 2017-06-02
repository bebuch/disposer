#include <disposer/input.hpp>


int success(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;32msuccess\033[0m\n";
	return 0;
}

int fail(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;31mfail\033[0m;\n";
	return 1;
}


template < typename T >
int check(std::size_t i, T const& test, T const& ref){
	if(test == ref){
		return success(i);
	}else{
		return fail(i);
	}
}


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::literals;

constexpr auto types = hana::tuple_t< int, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using hana::type_c;

	static constexpr disposer::iop_log iop_log{"pre", "input", "v"};
	static constexpr auto iops = hana::make_tuple();
	static constexpr auto get_object =
		disposer::iop_list< hana::tuple<> >(iop_log, iops);

	std::optional< disposer::output_info > const info;

	std::size_t ct = 0;
	std::size_t error_count = 0;

	try{
		{
			constexpr auto maker = "v"_in(hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int >,
					disposer::connection_verify_always,
					disposer::type_verify_always
				> const >);

			auto object = maker(get_object, nullptr, true, info);

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int > >);

			error_count = check(ct++, object.is_enabled(), false);
			error_count = check(ct++, object.is_enabled(type_c< int >), false);
		}

		{
			constexpr auto maker = "v"_in(types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int, float >,
					disposer::connection_verify_always,
					disposer::type_verify_always
				> const >);

			auto object = maker(get_object, nullptr, true, info);

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int, float > >);

			error_count = check(ct++, object.is_enabled(), false);
			error_count = check(ct++, object.is_enabled(type_c< int >), false);
		}

		{
			constexpr auto maker = "v"_in(types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int, float >,
					disposer::connection_verify_always,
					disposer::type_verify_always
				> const >);

			auto object = maker(get_object, nullptr, true, info);

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int, float > >);

			error_count = check(ct++, object.is_enabled(), false);
			error_count = check(ct++, object.is_enabled(type_c< int >), false);
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
