#include <disposer/parameter.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, float >;
constexpr auto types_set = hana::to_set(types);


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


int main(){
	using namespace std::literals::string_view_literals;
	using namespace hana::literals;
	using hana::type_c;

	static constexpr auto iops = hana::make_tuple();
	static constexpr auto get_object =
		disposer::iop_list< hana::tuple<> >(iops);

	std::size_t ct = 0;
	std::size_t error_count = 0;

	auto const value_int = hana::make_map(
		hana::make_pair(hana::type_c< int >, std::make_optional("5"sv))
	);

	auto const value_int_float = hana::make_map(
		hana::make_pair(hana::type_c< int >, std::make_optional("5"sv)),
		hana::make_pair(hana::type_c< float >, std::make_optional("5"sv))
	);

	try{
		{
			constexpr auto maker = "v"_param(hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					decltype("v"_param),
					disposer::parameter< decltype("v"_param), int >,
					disposer::value_verify_always,
					disposer::enable_always,
					disposer::stream_parser,
					decltype(hana::make_map(
						hana::make_pair(hana::type_c< int >, "sint32"_s)
					))
				> const >);

			auto object = maker(get_object, value_int);

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param), int > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object(),
				5
			);

			error_count = check(ct++,
				object(hana::type_c< int >),
				5
			);
		}

		{
			constexpr auto maker = "v"_param(types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					decltype("v"_param),
					disposer::parameter< decltype("v"_param), int, float >,
					disposer::value_verify_always,
					disposer::enable_always,
					disposer::stream_parser,
					decltype(hana::make_map(
						hana::make_pair(hana::type_c< int >, "sint32"_s),
						hana::make_pair(hana::type_c< float >, "float32"_s)
					))
				> const >);

			auto object = maker(get_object, value_int_float);

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param), int, float > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< float >),
				true
			);

			error_count = check(ct++,
				object(hana::type_c< int >),
				5
			);

			error_count = check(ct++,
				object(hana::type_c< float >),
				5.f
			);
		}

		{
			constexpr auto maker = "v"_param(types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					decltype("v"_param),
					disposer::parameter< decltype("v"_param), int, float >,
					disposer::value_verify_always,
					disposer::enable_always,
					disposer::stream_parser,
					decltype(hana::make_map(
						hana::make_pair(hana::type_c< int >, "sint32"_s),
						hana::make_pair(hana::type_c< float >, "float32"_s)
					))
				> const >);

			auto object = maker(get_object, value_int_float);

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param), int, float > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< float >),
				true
			);

			error_count = check(ct++,
				object(hana::type_c< int >),
				5
			);

			error_count = check(ct++,
				object(hana::type_c< float >),
				5.f
			);
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
