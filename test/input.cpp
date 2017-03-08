#include <disposer/input.hpp>

namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::interface::module;


// struct M1{
// 	decltype(hana::make_map(
// 		input("test1"_s, hana::type_c< int >)
// 	)) in;
// };

// struct M2{
// 	decltype(hana::make_map(
// 		input("test1"_s, hana::type_c< int >),
// 		input("test2"_s, hana::type_c< char >),
// 		input("test3"_s, hana::type_c< float >)
// 	)) in;
// };
//
// constexpr auto types = hana::tuple_t< int, char, float >;
//
// struct M3{
// 	decltype(hana::make_map(
// 		input("test1"_s, types)
// 	)) in;
// };
//
//
// constexpr auto types_set = hana::to_set(types);
//
// struct M4{
// 	decltype(hana::make_map(
// 		input("test1"_s, types_set)
// 	)) in;
// };
//
//
// struct M5{
// 	decltype(hana::make_map(
// 		input("test1"_s, hana::type_c< int >),
// 		input("test2"_s, types),
// 		input("test3"_s, types_set)
// 	)) in;
// };



int main(){
	auto i = input("test1"_s, hana::type_c< int >);
// 	M1 m1;
// 	M2 m2;
// 	M3 m3;
// 	M4 m4;
// 	M5 m5;
}
