#include <disposer/output.hpp>
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
using namespace std::literals::string_view_literals;

constexpr auto types = hana::tuple_t< int, float >;
constexpr auto types_set = hana::to_set(types);


int main(){
	using hana::type_c;

	std::size_t ct = 0;
	std::size_t error_count = 0;

	auto iop_list = hana::make_tuple();
	auto output_make_data = [&iop_list](auto const& maker){
		auto make_data = disposer::output_make_data(maker);
		disposer::iops_make_data data(
				std::move(make_data), "location"sv, iop_list, hana::size_c< 0 >
			);

		return data;
	};
	constexpr auto output_maker = "v"_out(hana::type_c< int >);
	using output_type = decltype(hana::typeid_(output_maker))::type::type;
	output_type output(output_make_data(output_maker));

	auto make_data = [&iop_list, &output](auto const& maker){
		auto make_data = disposer::input_make_data(
			maker, disposer::output_info(&output, true));
		disposer::iops_make_data data(
				std::move(make_data), "location"sv, iop_list, hana::size_c< 0 >
			);

		return data;
	};

	try{
		{
			constexpr auto maker = "v"_in(hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int >,
					disposer::required_t,
					disposer::type_verify_always
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int > >);

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
		}

		{
			constexpr auto maker = "v"_in(types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int, float >,
					disposer::required_t,
					disposer::type_verify_always
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int, float > >);

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
		}

		{
			constexpr auto maker = "v"_in(types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::input_maker<
					disposer::input< decltype("v"_in),
						disposer::no_transform, int, float >,
					disposer::required_t,
					disposer::type_verify_always
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::input< decltype("v"_in),
					disposer::no_transform, int, float > >);

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
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
