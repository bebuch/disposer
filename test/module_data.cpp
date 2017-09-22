#include <disposer/core/module_data.hpp>


namespace disposer{


	template < typename ... RefList >
	module_data(hana::tuple< RefList ... >&&)
		-> module_data<
			type_list<>,
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< input_tag >)),
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< output_tag >)),
			decltype(hana::filter(
				std::declval< hana::tuple< RefList ... >&& >(),
				hana::is_a< parameter_tag >)) >;

}

using namespace disposer;
using namespace disposer::literals;


int main(){
	output< decltype("o1"_out), int > o1{1};
	input< decltype("i1"_in), long, false > i1{nullptr};
	input< decltype("i2"_in), int, true > i2{&o1};
	parameter< decltype("p1"_param), int > p1{5};
	parameter< decltype("p2"_param), char > p2{'x'};
	output< decltype("o2"_out), bool > o2{0};

	{
		auto list = iops_ref{};
		module_data data(std::move(list).flat());
		static_assert(std::is_same_v< decltype(data),
			module_data< type_list<>,
				hana::tuple<>, hana::tuple<>, hana::tuple<> > >);
	}
	{
		auto list = iops_ref{
			std::move(i2), iops_ref{std::move(i1), iops_ref{}}};
		module_data data(std::move(list).flat());
		static_assert(std::is_same_v< decltype(data), module_data<
				type_list<>,
				hana::tuple<
					input< decltype("i1"_in), long, false >,
					input< decltype("i2"_in), int, true >
				>,
				hana::tuple<>,
				hana::tuple<>
			> >);
	}
	{
		auto list = iops_ref{
			std::move(o2), iops_ref{std::move(o1), iops_ref{}}};
		module_data data(std::move(list).flat());
		static_assert(std::is_same_v< decltype(data), module_data<
				type_list<>,
				hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), int >,
					output< decltype("o2"_out), bool >
				>,
				hana::tuple<>
			> >);
	}
	{
		auto list = iops_ref{
			std::move(p2), iops_ref{std::move(p1), iops_ref{}}};
		module_data data(std::move(list).flat());
		static_assert(std::is_same_v< decltype(data), module_data<
				type_list<>,
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<
					parameter< decltype("p1"_param), int >,
					parameter< decltype("p2"_param), char >
				>
			> >);
	}
	{
		auto list = iops_ref{std::move(o2), iops_ref{std::move(p2),
			iops_ref{std::move(p1), iops_ref{std::move(i2),
			iops_ref{std::move(i1), iops_ref{std::move(o1), iops_ref{}}}}}}};
		module_data data(std::move(list).flat());
		static_assert(std::is_same_v< decltype(data), module_data<
				type_list<>,
				hana::tuple<
					input< decltype("i1"_in), long, false >,
					input< decltype("i2"_in), int, true >
				>,
				hana::tuple<
					output< decltype("o1"_out), int >,
					output< decltype("o2"_out), bool >
				>,
				hana::tuple<
					parameter< decltype("p1"_param), int >,
					parameter< decltype("p2"_param), char >
				>
			> >);
	}
}
