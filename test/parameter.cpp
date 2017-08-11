#include <disposer/core/parameter_maker.hpp>
#include <disposer/core/accessory.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::literals;

using disposer::make;

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

	std::size_t ct = 0;
	std::size_t error_count = 0;

	auto const value_int = hana::make_map(
		hana::make_pair(hana::type_c< int >, std::make_optional("5"sv))
	);

	auto const value_int_float = hana::make_map(
		hana::make_pair(hana::type_c< int >, std::make_optional("5"sv)),
		hana::make_pair(hana::type_c< float >, std::make_optional("5"sv))
	);

	auto iop_list = hana::make_tuple();
	auto make_data = [&iop_list](auto const& maker, auto const& value_map){
		auto make_data = disposer::parameter_make_data(maker, value_map);
		disposer::iops_make_data data(
				std::move(make_data), "location"sv, iop_list
			);

		return data;
	};

	try{
		{
			constexpr auto maker = make("v"_param, hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					disposer::parameter< decltype("v"_param),
						disposer::none, int >,
					disposer::verify_value_always_t,
					disposer::enable_always_t,
					disposer::stream_parser_t,
					disposer::auto_default_t,
					decltype(hana::make_map(
						hana::make_pair(hana::basic_type< int >{}, "sint32"_s)
					))
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker, value_int));

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param),
					disposer::none, int > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object.get(),
				5
			);

			error_count = check(ct++,
				object.get(hana::type_c< int >),
				5
			);
		}

		{
			constexpr auto maker = make("v"_param, types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					disposer::parameter< decltype("v"_param),
						disposer::none, int, float >,
					disposer::verify_value_always_t,
					disposer::enable_always_t,
					disposer::stream_parser_t,
					disposer::auto_default_t,
					decltype(hana::make_map(
						hana::make_pair(hana::basic_type< int >{}, "sint32"_s),
						hana::make_pair(hana::basic_type< float >{}, "float32"_s)
					))
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker, value_int_float));

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param),
					disposer::none, int, float > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< float >),
				true
			);

			error_count = check(ct++,
				object.get(hana::type_c< int >),
				5
			);

			error_count = check(ct++,
				object.get(hana::type_c< float >),
				5.f
			);
		}

		{
			constexpr auto maker = make("v"_param, types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::parameter_maker<
					disposer::parameter< decltype("v"_param),
						disposer::none, int, float >,
					disposer::verify_value_always_t,
					disposer::enable_always_t,
					disposer::stream_parser_t,
					disposer::auto_default_t,
					decltype(hana::make_map(
						hana::make_pair(hana::basic_type< int >{}, "sint32"_s),
						hana::make_pair(hana::basic_type< float >{}, "float32"_s)
					))
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker, value_int_float));

			static_assert(std::is_same_v< decltype(object),
				disposer::parameter< decltype("v"_param),
					disposer::none, int, float > >);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< int >),
				true
			);

			error_count = check(ct++,
				object.is_enabled(hana::type_c< float >),
				true
			);

			error_count = check(ct++,
				object.get(hana::type_c< int >),
				5
			);

			error_count = check(ct++,
				object.get(hana::type_c< float >),
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
