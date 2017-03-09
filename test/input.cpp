#include <disposer/input.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;


struct M1{
	decltype(io_list(
		in("test1"_s, hana::type_c< int >)
	)) io;
};

struct M2{
	decltype(io_list(
		in("test1"_s, hana::type_c< int >),
		in("test2"_s, hana::type_c< char >),
		in("test3"_s, hana::type_c< float >)
	)) io;
};

constexpr auto types = hana::tuple_t< int, char, float >;

struct M3{
	decltype(io_list(
		in("test1"_s, types)
	)) io;
};

//
// constexpr auto types_set = hana::to_set(types);
//
// struct M4{
// 	decltype(io_list(
// 		in("test1"_s, types_set)
// 	)) io;
// };
//
//
// struct M5{
// 	decltype(io_list(
// 		in("test1"_s, hana::type_c< int >),
// 		in("test2"_s, types),
// 		in("test3"_s, types_set)
// 	)) io;
// };



int main(){
	M1 m1;
	M2 m2;
// 	M3 m3;
// 	M4 m4;
// 	M5 m5;
}
