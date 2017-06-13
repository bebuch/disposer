#include <disposer/module.hpp>

#include <iostream>
#include <iomanip>


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::literals;

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
	std::size_t error_count = 0;

	try{
		disposer::disposer program;
		auto& declarant = program.declarant();

		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(),
				disposer::normal_id_increase(),
				[](auto const& module){
					module.log([](logsys::stdlogb&){});
					return [](auto& module, std::size_t){
						module.log([](logsys::stdlogb&){});
					};
				}
			);
			module_register_fn("m1", declarant);
		}

		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >)
				),
				disposer::normal_id_increase(),
				[](auto const& config){
					auto valid_type =
						hana::type_c< decltype(config("v"_in)) >
						== hana::type_c< disposer::input< decltype("v"_in),
							disposer::no_transform, int > const& >;
					static_assert(valid_type);

					return [](auto& module, std::size_t){
						auto valid_type =
							hana::type_c< decltype(module("v"_in)) >
							== hana::type_c< disposer::input< decltype("v"_in),
								disposer::no_transform, int >& >;
						static_assert(valid_type);

						auto const& const_module = module;
						auto valid_const_type =
							hana::type_c< decltype(const_module("v"_in)) >
							== hana::type_c< disposer::input< decltype("v"_in),
								disposer::no_transform, int > const& >;
						static_assert(valid_const_type);

						auto valid_value_type =  hana::type_c<
							decltype(module("v"_in).get_values()) > ==
							hana::type_c< std::multimap< std::size_t, int > >;
						static_assert(valid_value_type );

						auto valid_ref_type =  hana::type_c<
							decltype(module("v"_in).get_references()) > ==
							hana::type_c< std::multimap< std::size_t,
								std::reference_wrapper< int const> > >;
						static_assert(valid_ref_type);
					};
				}
			);
			module_register_fn("m2", declarant);
		}


		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(
					"v"_out(hana::type_c< int >)
				),
				disposer::normal_id_increase(),
				[](auto const&){ return [](auto&, std::size_t){}; }
			);
			module_register_fn("m3", declarant);
		}


		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(
					"v"_param(hana::type_c< int >)
				),
				disposer::normal_id_increase(),
				[](auto const&){ return [](auto&, std::size_t){}; }
			);
			module_register_fn("m4", declarant);
		}

		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >),
					"v"_out(hana::type_c< int >),
					"v"_param(hana::type_c< int >)
				),
				disposer::normal_id_increase(),
				[](auto const&){ return [](auto&, std::size_t){}; }
			);
			module_register_fn("m5", declarant);
		}

		{
			auto module_register_fn = disposer::make_module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >),
					"v"_out(hana::type_c< int >,
						disposer::type_transform(disposer::no_transform{}),
						disposer::enable([](auto const& get, auto type){
							bool active = get("v"_in).is_enabled(type);
							assert(!active);
							return active;
						})),
					"v"_param(hana::type_c< int >,
						disposer::value_verify(disposer::value_verify_always()),
						disposer::enable([](auto const& get, auto type){
							bool active1 = get("v"_in).is_enabled(type);
							bool active2 = get("v"_out).is_enabled(type);
							assert(!active1 && !active2);
							return false;
						}),
						disposer::parser([](
							auto const& /*iop*/, std::string_view, auto type
						){
							static_assert(type == hana::type_c< int >);
							return 5;
						}),
						disposer::default_values(7)),
					"w"_in(hana::type_c< int >,
						disposer::type_transform(disposer::no_transform{}),
						disposer::connection_verify(
							[](auto const&, bool connected){
								assert(!connected);
							}),
						disposer::type_verify(
							[](
								auto const& get,
								auto type,
								disposer::output_info const&
							){
								bool active1 = get("v"_in).is_enabled(type);
								bool active2 = get("v"_out).is_enabled(type);
								bool active3 = get("v"_param).is_enabled(type);
								assert(!active1 && !active2 && !active3);
							}))
				),
				disposer::normal_id_increase(),
				[](auto const&){ return [](auto&, std::size_t){}; }
			);
			module_register_fn("m6", declarant);
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
