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



struct exec_fn{
	template < typename T >
	constexpr void operator()(T& /*module*/, std::size_t /*id*/)const{}
};

struct enable_fn{
	template < typename T >
	constexpr auto operator()(T const& /*module*/)const{
		return exec_fn{};
	}
};

struct disposer::unit_test_key{
	template < typename Register >
	constexpr auto copy(Register const& reg)const{
		return (reg.maker_);
	}

	template < typename Register >
	constexpr decltype(auto) ref(Register const& reg)const{
		return (reg.maker_);
	}
};

int main(){
	using namespace hana::literals;

	static constexpr auto key = disposer::unit_test_key{};

	std::size_t error_count = 0;

	try{
		{
			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v< decltype(
				key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<>, enable_fn
				> > >);
		}

		{
			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >, disposer::optional)
				),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v<
				decltype(key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
					disposer::input_maker<
						disposer::input< decltype("v"_in),
							disposer::none, int >,
						disposer::optional_t,
						disposer::type_verify_always_t
					>
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<
						disposer::input< decltype("v"_in),
							disposer::none, int >
					>, enable_fn
				> > >);
		}


		{
			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_out(hana::type_c< int >)
				),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v<
				decltype(key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
					disposer::output_maker<
						disposer::output< decltype("v"_out),
							disposer::none, int >,
						disposer::enable_always_t
					>
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<
						disposer::output< decltype("v"_out),
							disposer::none, int >
					>, enable_fn
				> > >);
		}


		{
			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_param(hana::type_c< int >)
				),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v<
				decltype(key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
					disposer::parameter_maker<
						disposer::parameter< decltype("v"_param),
							disposer::none, int >,
							disposer::value_verify_always_t,
							disposer::enable_always_t,
							disposer::stream_parser_t,
							disposer::auto_default_t,
						decltype(hana::make_map(
							hana::make_pair(hana::basic_type< int >{}, "sint32"_s)
						))
					>
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{
					{}, {}, {}, {}, {}, {{"v", {"value", {}}}}
				});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<
						disposer::parameter< decltype("v"_param),
							disposer::none, int >
					>, enable_fn
				> > >);
		}

		{
			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >, disposer::optional),
					"v"_out(hana::type_c< int >),
					"v"_param(hana::type_c< int >)
				),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v<
				decltype(key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
					disposer::input_maker<
						disposer::input< decltype("v"_in),
							disposer::none, int >,
						disposer::optional_t,
						disposer::type_verify_always_t
					>,
					disposer::output_maker<
						disposer::output< decltype("v"_out),
							disposer::none, int >,
						disposer::enable_always_t
					>,
					disposer::parameter_maker<
						disposer::parameter< decltype("v"_param),
							disposer::none, int >,
							disposer::value_verify_always_t,
							disposer::enable_always_t,
							disposer::stream_parser_t,
							disposer::auto_default_t,
						decltype(hana::make_map(
							hana::make_pair(hana::basic_type< int >{}, "sint32"_s)
						))
					>
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{
					{}, {}, {}, {}, {}, {{"v", {"value", {}}}}
				});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<
						disposer::input< decltype("v"_in),
							disposer::none, int >,
						disposer::output< decltype("v"_out),
							disposer::none, int >,
						disposer::parameter< decltype("v"_param),
							disposer::none, int >
					>, enable_fn
				> > >);
		}

		{
			constexpr auto enable_out = [](auto const& get, auto type){
				bool active = get("v"_in).is_enabled(type);
				assert(!active);
				return active;
			};

			constexpr auto enable_in_c = [](auto const&, bool connected){
				(void)connected;
				assert(!connected);
			};

			constexpr auto enable_param = [](auto const& get, auto type){
				bool active1 = get("v"_in).is_enabled(type);
				bool active2 = get("v"_out).is_enabled(type);
				(void)active1, (void)active2;
				assert(!active1 && !active2);
				return false;
			};

			constexpr auto parser =
				[](auto const& /*iop*/, std::string_view /*value*/, auto type){
					static_assert(type == hana::type_c< int >);
					return 5;
				};

			constexpr auto default_value =
				[](auto const&, auto){
					return 7;
				};

			constexpr auto enable_in_t =
				[](auto const& get, auto type, disposer::output_info const&){
					bool active1 = get("v"_in).is_enabled(type);
					bool active2 = get("v"_out).is_enabled(type);
					bool active3 = get("v"_param).is_enabled(type);
					(void)active1, (void)active2, (void)active3;
					assert(!active1 && !active2 && !active3);
				};

			auto const module_register_fn = disposer::module_register_fn(
				disposer::module_configure(
					"v"_in(hana::type_c< int >, disposer::optional),
					"v"_out(hana::type_c< int >,
						disposer::no_type_transform,
						disposer::enable_fn(enable_out)),
					"v"_param(hana::type_c< int >,
						disposer::value_verify_always,
						disposer::enable_fn(enable_param),
						disposer::parser_fn(parser),
						disposer::default_value_fn(default_value)),
					"w"_in(hana::type_c< int >,
						disposer::no_type_transform,
						disposer::connection_verify_fn(enable_in_c),
						disposer::type_verify_fn(enable_in_t))
				),
				disposer::module_enable(enable_fn())
			);

			static_assert(std::is_same_v<
				decltype(key.copy(module_register_fn)),
				disposer::module_maker< hana::tuple<
					disposer::input_maker<
						disposer::input< decltype("v"_in),
							disposer::none, int >,
						disposer::optional_t,
						disposer::type_verify_always_t
					>,
					disposer::output_maker<
						disposer::output< decltype("v"_out),
							disposer::none, int >,
						std::remove_const_t< decltype(enable_out) >
					>,
					disposer::parameter_maker<
						disposer::parameter< decltype("v"_param),
							disposer::none, int >,
						disposer::value_verify_always_t,
						std::remove_const_t< decltype(enable_param) >,
						std::remove_const_t< decltype(parser) >,
						std::remove_const_t< decltype(default_value) >,
						decltype(hana::make_map(
							hana::make_pair(hana::basic_type< int >{}, "sint32"_s)
						))
					>,
					disposer::input_maker<
						disposer::input< decltype("w"_in),
							disposer::none, int >,
						std::remove_const_t< decltype(enable_in_c) >,
						std::remove_const_t< decltype(enable_in_t) >
					>
				>, enable_fn > >);

			auto object =
				key.ref(module_register_fn)(disposer::module_make_data{
					{}, {}, {}, {}, {}, {{"v", {"value", {}}}}
				});

			static_assert(std::is_same_v< decltype(object),
				std::unique_ptr< disposer::module<
					hana::tuple<
						disposer::input< decltype("v"_in),
							disposer::none, int >,
						disposer::output< decltype("v"_out),
							disposer::none, int >,
						disposer::parameter< decltype("v"_param),
							disposer::none, int >,
						disposer::input< decltype("w"_in),
							disposer::none, int >
					>, enable_fn
				> > >);
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
