#include <disposer/module.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;

constexpr auto types = hana::tuple_t< int, char, float >;


void check_impl(std::string_view name, bool enabled, bool expected){
	if(enabled == expected){
		std::cout << " \033[0;32msuccess:\033[0m ";
	}else{
		std::cout << " \033[0;31mfail:\033[0m ";
	}
	std::cout << name << " = " << std::boolalpha << expected << "\n";
}

template < typename IO >
void check(IO& io, bool expected){
	check_impl(io.name, io.is_enabled(), expected);
}

template < typename IO, typename Type >
void check(IO& io, Type const& type, bool expected){
	check_impl(io.name, io.is_enabled(type), expected);
}


int main(){
	using ident =
		decltype(hana::typeid_(hana::template_< disposer::self_t >))::type;

	std::size_t error_count = 0;

	try{
		disposer::disposer program;
		auto& declarant = program.declarant();

		{
			auto maker = "m1"_module();

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
		}

		{
			auto maker = "m2"_module(
				"v"_in(hana::type_c< int >)
			);

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
		}


		{
			auto maker = "m3"_module(
				"v"_out(hana::type_c< int >)
			);

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
		}


		{
			auto maker = "m4"_module(
				"v"_param(hana::type_c< int >)
			);

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
		}

		{
			auto maker = "m5"_module(
				"v"_in(hana::type_c< int >),
				"v"_out(hana::type_c< int >),
				"v"_param(hana::type_c< int >)
			);

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
		}

		{
			constexpr auto enable_out = [](auto const& get, auto type){
				bool active = get("v"_in).is_enabled(type);
				assert(!active);
				return active;
			};

			constexpr auto enable_in_c = [](auto const&, bool connected){
				assert(!connected);
			};

			constexpr auto enable_param = [](auto const& get, auto type){
				bool active1 = get("v"_in).is_enabled(type);
				bool active2 = get("v"_out).is_enabled(type);
				assert(!active1 && !active2);
				return false;
			};

			constexpr auto parser = [](std::string const& /*value*/, auto type){
				static_assert(type == hana::type_c< int >);
				return 5;
			};

			constexpr auto enable_in_t =
				[](auto const& get, auto type, disposer::output_info const&){
					bool active1 = get("v"_in).is_enabled(type);
					bool active2 = get("v"_out).is_enabled(type);
					bool active3 = get("v"_param).is_enabled(type);
					assert(!active1 && !active2 && !active3);
				};

			auto maker = "m6"_module(
				"v"_in(hana::type_c< int >),
				"v"_out(hana::type_c< int >, ident{}, enable_out),
				"v"_param(hana::type_c< int >, enable_param, parser,
					std::make_tuple(7)),
				"w"_in(hana::type_c< int >, ident{}, enable_in_c, enable_in_t)
			);

			auto register_fn = disposer::make_register_fn(std::move(maker));
			register_fn(declarant);
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
