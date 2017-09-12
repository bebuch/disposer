#include <disposer/core/make_module.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


namespace hana = boost::hana;

using namespace disposer;

using namespace hana::literals;
using namespace disposer::literals;
using namespace std::literals::string_view_literals;


using type_index = boost::typeindex::ctti_type_index;

constexpr auto list = dimension_list{
		dimension_c< double, char, float >,
		dimension_c< int, bool >,
		dimension_c< short, unsigned, long, long long >
	};

constexpr auto list_size = hana::size(list.dimensions).value;


template < std::size_t D >
using ic = index_component< D >;


constexpr void exec(){}
constexpr state_maker_fn state_dummy{};
constexpr exec_fn exec_dummy{&exec};


// template < typename T, typename Maker, std::size_t ... D >
// int check_not_required(
// 	std::size_t i,
// 	hana::basic_type< T >,
// 	Maker const& maker,
// 	ic< D > const& ... ics
// ){
// 	module_make_data module_data{};
//
// 	auto const data = make_construct_data(maker, list, module_data,
// 		make_list_index< list_size >(ics ...), hana::make_tuple());
//
// 	if(std::holds_alternative< input_construct_data<
// 		decltype("v"_in), T, false > >(hana::second(data[0_c]))
// 	){
// 		return success(i);
// 	}else{
// 		return fail(i);
// 	}
// }
//
// template < typename T, typename Maker, std::size_t ... D >
// int check_not_deduced(
// 	std::size_t i,
// 	hana::basic_type< T >,
// 	Maker const& maker,
// 	ic< D > const& ... ics
// ){
// 	output< decltype("v"_out), T > out(1);
//
// 	module_make_data module_data{{}, {}, {}, {{"v", &out}}, {}, {}};
//
// 	auto const data = make_construct_data(maker, list, module_data,
// 		make_list_index< list_size >(ics ...), hana::make_tuple());
//
// 	if(std::holds_alternative< input_construct_data<
// 		decltype("v"_in), T, true > >(hana::second(data[0_c]))
// 	){
// 		return success(i);
// 	}else{
// 		return fail(i);
// 	}
// }
//
// template < typename T, typename Maker >
// int check_deduced(
// 	std::size_t i,
// 	hana::basic_type< T >,
// 	Maker const& maker
// ){
// 	output< decltype("v"_out), T > out(1);
//
// 	module_make_data module_data{{}, {}, {}, {{"v", &out}}, {}, {}};
//
// 	auto const data = make_construct_data(maker, list, module_data,
// 		make_list_index< list_size >(), hana::make_tuple());
//
// 	if(std::holds_alternative< input_construct_data<
// 		decltype("v"_in), T, true > >(hana::second(data[0_c]))
// 	){
// 		return success(i);
// 	}else{
// 		return fail(i);
// 	}
// }


template < typename ... T > struct morph{};


BOOST_AUTO_TEST_CASE(test_1){
	module_make_data module_data{};
	auto const module_base_ptr = make_module_ptr(list,
		module_configure{make("v"_in, type_ref_c< 0 >, not_required)},
		module_data, "input_tests", state_dummy, exec_dummy);
}


// int main(){
// 	std::size_t ct = 0;
// 	std::size_t ec = 0;
//
//
// 	try{
// 		{
// 			ec = check_not_required(ct++, type_c< double >,
// 				maker, ic< 0 >{0});
// 			ec = check_not_required(ct++, type_c< char >,
// 				maker, ic< 0 >{1});
// 			ec = check_not_required(ct++, type_c< float >,
// 				maker, ic< 0 >{2});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 1 >, not_required);
// 			ec = check_not_required(ct++, type_c< int >,
// 				maker, ic< 1 >{0});
// 			ec = check_not_required(ct++, type_c< bool >,
// 				maker, ic< 1 >{1});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 2 >, not_required);
// 			ec = check_not_required(ct++, type_c< short >,
// 				maker, ic< 2 >{0});
// 			ec = check_not_required(ct++, type_c< unsigned >,
// 				maker, ic< 2 >{1});
// 			ec = check_not_required(ct++, type_c< long >,
// 				maker, ic< 2 >{2});
// 			ec = check_not_required(ct++, type_c< long long >,
// 				maker, ic< 2 >{3});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 0, 1 >, not_required);
// 			ec = check_not_required(ct++, type_c< morph< double, int > >,
// 				maker, ic< 0 >{0}, ic< 1 >{0});
// 			ec = check_not_required(ct++, type_c< morph< char, int > >,
// 				maker, ic< 0 >{1}, ic< 1 >{0});
// 			ec = check_not_required(ct++, type_c< morph< float, int > >,
// 				maker, ic< 0 >{2}, ic< 1 >{0});
// 			ec = check_not_required(ct++, type_c< morph< double, bool > >,
// 				maker, ic< 0 >{0}, ic< 1 >{1});
// 			ec = check_not_required(ct++, type_c< morph< char, bool > >,
// 				maker, ic< 0 >{1}, ic< 1 >{1});
// 			ec = check_not_required(ct++, type_c< morph< float, bool > >,
// 				maker, ic< 0 >{2}, ic< 1 >{1});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 2, 0 >, not_required);
// 			ec = check_not_required(ct++, type_c< morph< short, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{0});
// 			ec = check_not_required(ct++, type_c< morph< unsigned, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{1});
// 			ec = check_not_required(ct++, type_c< morph< long, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{2});
// 			ec = check_not_required(ct++, type_c< morph< long long, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{3});
// 			ec = check_not_required(ct++, type_c< morph< short, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{0});
// 			ec = check_not_required(ct++, type_c< morph< unsigned, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{1});
// 			ec = check_not_required(ct++, type_c< morph< long, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{2});
// 			ec = check_not_required(ct++, type_c< morph< long long, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{3});
// 			ec = check_not_required(ct++, type_c< morph< short, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{0});
// 			ec = check_not_required(ct++, type_c< morph< unsigned, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{1});
// 			ec = check_not_required(ct++, type_c< morph< long, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{2});
// 			ec = check_not_required(ct++, type_c< morph< long long, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{3});
// 		}
//
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 0 >);
// 			ec = check_not_deduced(ct++, type_c< double >,
// 				maker, ic< 0 >{0});
// 			ec = check_not_deduced(ct++, type_c< char >,
// 				maker, ic< 0 >{1});
// 			ec = check_not_deduced(ct++, type_c< float >,
// 				maker, ic< 0 >{2});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 1 >);
// 			ec = check_not_deduced(ct++, type_c< int >,
// 				maker, ic< 1 >{0});
// 			ec = check_not_deduced(ct++, type_c< bool >,
// 				maker, ic< 1 >{1});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 2 >);
// 			ec = check_not_deduced(ct++, type_c< short >,
// 				maker, ic< 2 >{0});
// 			ec = check_not_deduced(ct++, type_c< unsigned >,
// 				maker, ic< 2 >{1});
// 			ec = check_not_deduced(ct++, type_c< long >,
// 				maker, ic< 2 >{2});
// 			ec = check_not_deduced(ct++, type_c< long long >,
// 				maker, ic< 2 >{3});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 0, 1 >);
// 			ec = check_not_deduced(ct++, type_c< morph< double, int > >,
// 				maker, ic< 0 >{0}, ic< 1 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< char, int > >,
// 				maker, ic< 0 >{1}, ic< 1 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< float, int > >,
// 				maker, ic< 0 >{2}, ic< 1 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< double, bool > >,
// 				maker, ic< 0 >{0}, ic< 1 >{1});
// 			ec = check_not_deduced(ct++, type_c< morph< char, bool > >,
// 				maker, ic< 0 >{1}, ic< 1 >{1});
// 			ec = check_not_deduced(ct++, type_c< morph< float, bool > >,
// 				maker, ic< 0 >{2}, ic< 1 >{1});
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 2, 0 >);
// 			ec = check_not_deduced(ct++, type_c< morph< short, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< unsigned, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{1});
// 			ec = check_not_deduced(ct++, type_c< morph< long, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{2});
// 			ec = check_not_deduced(ct++, type_c< morph< long long, double > >,
// 				maker, ic< 0 >{0}, ic< 2 >{3});
// 			ec = check_not_deduced(ct++, type_c< morph< short, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< unsigned, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{1});
// 			ec = check_not_deduced(ct++, type_c< morph< long, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{2});
// 			ec = check_not_deduced(ct++, type_c< morph< long long, char > >,
// 				maker, ic< 0 >{1}, ic< 2 >{3});
// 			ec = check_not_deduced(ct++, type_c< morph< short, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{0});
// 			ec = check_not_deduced(ct++, type_c< morph< unsigned, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{1});
// 			ec = check_not_deduced(ct++, type_c< morph< long, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{2});
// 			ec = check_not_deduced(ct++, type_c< morph< long long, float > >,
// 				maker, ic< 0 >{2}, ic< 2 >{3});
// 		}
//
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 0 >);
// 			ec = check_deduced(ct++, type_c< double >, maker);
// 			ec = check_deduced(ct++, type_c< char >, maker);
// 			ec = check_deduced(ct++, type_c< float >, maker);
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 1 >);
// 			ec = check_deduced(ct++, type_c< int >, maker);
// 			ec = check_deduced(ct++, type_c< bool >, maker);
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in, type_ref_c< 2 >);
// 			ec = check_deduced(ct++, type_c< short >, maker);
// 			ec = check_deduced(ct++, type_c< unsigned >, maker);
// 			ec = check_deduced(ct++, type_c< long >, maker);
// 			ec = check_deduced(ct++, type_c< long long >, maker);
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 0, 1 >);
// 			ec = check_deduced(ct++, type_c< morph< double, int > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< char, int > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< float, int > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< double, bool > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< char, bool > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< float, bool > >,
// 				maker);
// 		}
//
// 		{
// 			constexpr auto maker = make("v"_in,
// 				wrapped_type_ref_c< morph, 2, 0 >);
// 			ec = check_deduced(ct++, type_c< morph< short, double > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< unsigned, double > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long, double > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long long, double > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< short, char > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< unsigned, char > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long, char > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long long, char > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< short, float > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< unsigned, float > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long, float > >,
// 				maker);
// 			ec = check_deduced(ct++, type_c< morph< long long, float > >,
// 				maker);
// 		}
//
//
// 		if(ec == 0){
// 			std::cout << "\033[0;32mSUCCESS\033[0m\n";
// 		}else{
// 			std::cout << "\033[0;31mFAILS:\033[0m " << ec << '\n';
// 		}
// 	}catch(std::exception const& e){
// 		std::cerr << "Unexpected exception: " << e.what() << '\n';
// 	}catch(...){
// 		std::cerr << "Unexpected unknown exception\n";
// 	}
//
// 	return ec;
// }
