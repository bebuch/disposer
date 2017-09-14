#include <disposer/core/module.hpp>


using namespace disposer;
using namespace disposer::literals;

struct exec{
	void operator()()const{}
};


int main(){
	output< decltype("o1"_out), int > o1{1};
	input< decltype("i1"_in), long, false > i1{nullptr};
	input< decltype("i2"_in), int, true > i2{&o1};
	parameter< decltype("p1"_param), int > p1{5};
	parameter< decltype("p2"_param), char > p2{'x'};
	output< decltype("o2"_out), bool > o2{0};

	std::string const chain = "chain";
	std::string const type_name = "type";
	std::size_t const number = 4;

	exec_fn< exec > e{};
	state_maker_fn< void > s{};

	{
		auto list = iops_ref{};
		module m(chain, type_name, number, std::move(list).flat(), s, e);
		static_assert(std::is_same_v< decltype(m),
			module<
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<>,
				void,
				exec
			> >);
	}
	{
		auto list = iops_ref{
			std::move(i2), iops_ref{std::move(i1), iops_ref{}}};
		module m(chain, type_name, number, std::move(list).flat(), s, e);
		static_assert(std::is_same_v< decltype(m), module<
				hana::tuple<
					input< decltype("i1"_in), long, false >,
					input< decltype("i2"_in), int, true >
				>,
				hana::tuple<>,
				hana::tuple<>,
				void,
				exec
			> >);
	}
	{
		auto list = iops_ref{
			std::move(o2), iops_ref{std::move(o1), iops_ref{}}};
		module m(chain, type_name, number, std::move(list).flat(), s, e);
		static_assert(std::is_same_v< decltype(m), module<
				hana::tuple<>,
				hana::tuple<
					output< decltype("o1"_out), int >,
					output< decltype("o2"_out), bool >
				>,
				hana::tuple<>,
				void,
				exec
			> >);
	}
	{
		auto list = iops_ref{
			std::move(p2), iops_ref{std::move(p1), iops_ref{}}};
		module m(chain, type_name, number, std::move(list).flat(), s, e);
		static_assert(std::is_same_v< decltype(m), module<
				hana::tuple<>,
				hana::tuple<>,
				hana::tuple<
					parameter< decltype("p1"_param), int >,
					parameter< decltype("p2"_param), char >
				>,
				void,
				exec
			> >);
	}
	{
		auto list = iops_ref{std::move(o2), iops_ref{std::move(p2),
			iops_ref{std::move(p1), iops_ref{std::move(i2),
			iops_ref{std::move(i1), iops_ref{std::move(o1), iops_ref{}}}}}}};
		module m(chain, type_name, number, std::move(list).flat(), s, e);
		static_assert(std::is_same_v< decltype(m), module<
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
				>,
				void,
				exec
			> >);
	}
}
