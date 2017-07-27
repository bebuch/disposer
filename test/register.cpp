#include <disposer/component.hpp>
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
		auto& cdeclarant = program.component_declarant();
		auto& mdeclarant = program.module_declarant();

		{
			auto component_register_fn = disposer::component_register_fn(
				disposer::component_configure(),
				disposer::component_init([](auto const& component){
					component.log([](logsys::stdlogb&){});
					return 0;
				}),
				disposer::component_modules()
			);
			component_register_fn("c1", cdeclarant);
		}

		{
			auto component_register_fn = disposer::component_register_fn(
				disposer::component_configure(
					"v"_param(hana::type_c< int >)
				),
				disposer::component_init([](auto const& component){
					component.log([](logsys::stdlogb&){});
					return 0;
				}),
				disposer::component_modules()
			);
			component_register_fn("c2", cdeclarant);
		}

		{
			auto component_register_fn = disposer::component_register_fn(
				disposer::component_configure(
					"v"_param(hana::type_c< int >)
				),
				disposer::component_init([](auto const& component){
					component.log([](logsys::stdlogb&){});
					return 0;
				}),
				disposer::component_modules(
					"cm1"_module([](auto& /*component*/){
						return disposer::module_register_fn(
							disposer::module_configure(),
							disposer::module_enable([](auto const& module){
								module.log([](logsys::stdlogb&){});
								return [](auto& module, std::size_t){
									module.log([](logsys::stdlogb&){});
								};
							})
						);
					})
				)
			);
			component_register_fn("c3", cdeclarant);
		}

		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(),
				disposer::module_enable([](auto const& module){
					module.log([](logsys::stdlogb&){});
					return [](auto& module, std::size_t){
						module.log([](logsys::stdlogb&){});
					};
				})
			);
			module_register_fn("m1", mdeclarant);
		}

		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >)
				),
				disposer::module_enable([](auto const& config){
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
							hana::type_c< std::vector< int > >;
						static_assert(valid_value_type );

						auto valid_ref_type =  hana::type_c<
							decltype(module("v"_in).get_references()) > ==
							hana::type_c< std::vector<
								std::reference_wrapper< int const > > >;
						static_assert(valid_ref_type);
					};
				})
			);
			module_register_fn("m2", mdeclarant);
		}


		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_out(hana::type_c< int >)
				),
				disposer::module_enable([]{
					return [](auto& module){
						auto& out = module("v"_out);
						(void) out;
					};
				})
			);
			module_register_fn("m3", mdeclarant);
		}


		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_param(hana::type_c< int >)
				),
				disposer::module_enable([](auto const&){
					return [](auto&, std::size_t){};
				})
			);
			module_register_fn("m4", mdeclarant);
		}

		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >),
					"v"_out(hana::type_c< int >),
					"v"_param(hana::type_c< int >)
				),
				disposer::module_enable([](auto const&){
					return [](auto&, std::size_t){};
				})
			);
			module_register_fn("m5", mdeclarant);
		}

		{
			auto module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >),
					"v"_out(hana::type_c< int >,
						disposer::type_transform(disposer::no_transform{}),
						disposer::enable([](auto const& iop, auto type){
							bool active = iop("v"_in).is_enabled(type);
							assert(!active);
							return active;
						})),
					"v"_param(hana::type_c< int >,
						disposer::value_verify(disposer::value_verify_always()),
						disposer::enable([](auto const& iop, auto type){
							bool active1 = iop("v"_in).is_enabled(type);
							bool active2 = iop("v"_out).is_enabled(type);
							assert(!active1 && !active2);
							return false;
						}),
						disposer::parser([](
							auto const& /*iop*/, std::string_view, auto type
						){
							static_assert(type == hana::type_c< int >);
							return 5;
						}),
						disposer::default_value_fn([](auto const&, auto){
							return 7;
						})),
					"w"_in(hana::type_c< int >,
						disposer::type_transform(disposer::no_transform{}),
						disposer::connection_verify(
							[](auto const&, bool connected){
								assert(!connected);
							}),
						disposer::type_verify(
							[](
								auto const& iop,
								auto type,
								disposer::output_info const&
							){
								bool active1 = iop("v"_in).is_enabled(type);
								bool active2 = iop("v"_out).is_enabled(type);
								bool active3 = iop("v"_param).is_enabled(type);
								assert(!active1 && !active2 && !active3);
							}))
				),
				disposer::module_enable([](auto const&){
					return [](auto&, std::size_t){};
				})
			);
			module_register_fn("m6", mdeclarant);
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
