#include <disposer/output.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;


struct M1{
	decltype(hana::make_map(
		output("test1"_s, hana::type_c< int >)
	)) out;
};

struct M2{
	decltype(hana::make_map(
		output("test1"_s, hana::type_c< int >),
		output("test2"_s, hana::type_c< char >),
		output("test3"_s, hana::type_c< float >)
	)) out;
};

constexpr auto types = hana::tuple_t< int, char, float >;

struct M3{
	decltype(hana::make_map(
		output("test1"_s, types)
	)) out;
};


constexpr auto types_set = hana::to_set(types);

struct M4{
	decltype(hana::make_map(
		output("test1"_s, types_set)
	)) out;
};


struct M5{
	decltype(hana::make_map(
		output("test1"_s, hana::type_c< int >),
		output("test2"_s, types),
		output("test3"_s, types_set)
	)) out;
};



int main(){
	M1 m1;
	M2 m2;
	M3 m3;
	M4 m4;
	M5 m5;
}
