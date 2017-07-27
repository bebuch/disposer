#include <disposer/output.hpp>


int success(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;32msuccess\033[0m\n";
	return 0;
}

int fail(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;31mfail\033[0m;\n";
	return 1;
}


using type_index = boost::typeindex::ctti_type_index;


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

	disposer::module_base_key* key = nullptr;

	std::size_t ct = 0;
	std::size_t error_count = 0;

	auto iop_list = hana::make_tuple();
	auto make_data = [&iop_list](auto const& maker){
		auto make_data = disposer::output_make_data(maker);
		disposer::iops_make_data data(
				std::move(make_data), "location"sv, iop_list, hana::size_c< 0 >
			);

		return data;
	};

	try{
		{
			constexpr auto maker = "v"_out(hana::type_c< int >);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					disposer::output< decltype("v"_out),
						disposer::no_transform, int >,
					disposer::enable_always_t
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out),
					disposer::no_transform, int > >);

			object.output_base::prepare(std::move(*key), 0);
			object.put(0);

			error_count = check(ct++,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ type_index::type_id< int >(), true }
				});

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
		}

		{
			constexpr auto maker = "v"_out(types);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					disposer::output< decltype("v"_out),
						disposer::no_transform, int, float >,
					disposer::enable_always_t
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out),
					disposer::no_transform, int, float > >);

			object.output_base::prepare(std::move(*key), 0);
			object.put(3);
			object.put(3.f);

			error_count = check(ct++,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ type_index::type_id< int >(), true },
					{ type_index::type_id< float >(), true }
				});

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
			error_count = check(ct++, object.is_enabled(type_c< float >), true);
		}

		{
			constexpr auto maker = "v"_out(types_set);

			static_assert(std::is_same_v< decltype(maker),
				disposer::output_maker<
					disposer::output< decltype("v"_out),
						disposer::no_transform, int, float >,
					disposer::enable_always_t
				> const >);

			using type = decltype(hana::typeid_(maker))::type::type;
			type object(make_data(maker));

			static_assert(std::is_same_v< decltype(object),
				disposer::output< decltype("v"_out),
					disposer::no_transform, int, float > >);

			object.output_base::prepare(std::move(*key), 0);
			object.put(3);
			object.put(3.f);

			error_count = check(ct++,
				static_cast< disposer::output_base const& >(object)
					.enabled_types(),
				{
					{ type_index::type_id< int >(), true },
					{ type_index::type_id< float >(), true }
				});

			error_count = check(ct++, object.is_enabled(), true);
			error_count = check(ct++, object.is_enabled(type_c< int >), true);
			error_count = check(ct++, object.is_enabled(type_c< float >), true);
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
