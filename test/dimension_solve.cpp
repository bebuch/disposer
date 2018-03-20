#include <disposer/core/dimension_solve.hpp>

#include <boost/hana/string.hpp>

#define BOOST_TEST_MODULE disposer dimension_solve
#include <boost/test/included/unit_test.hpp>


constexpr std::size_t operator ""_z(unsigned long long n){ return n; }

template < typename ... T > struct morph{};

template < typename T1, typename T2, typename >
using ambiguous_31 = morph< T1, T2, long >;


struct some_type{};

template < typename T1, typename, typename >
using ambiguous_32 = morph< T1, some_type, some_type >;


using namespace disposer;
using namespace hana::literals;


template < std::size_t DI, std::size_t I >
constexpr auto ic = disposer::ct_index_component< DI, I >{};


auto list = dimension_list{
		dimension_c< double, char, float >,
		dimension_c< int, bool >,
		dimension_c< short, unsigned, long, long long >
	};

constexpr auto name = "input_name"_s;

BOOST_AUTO_TEST_CASE(test_1){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< double > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 0_z);
}

BOOST_AUTO_TEST_CASE(test_2){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< char > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_3){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< float > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 2_z);
}

BOOST_AUTO_TEST_CASE(test_4){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 1 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< int > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 0_z);
}

BOOST_AUTO_TEST_CASE(test_5){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 1 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_6){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< short > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 0_z);
}

BOOST_AUTO_TEST_CASE(test_7){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< unsigned > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_8){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< long > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 2_z);
}

BOOST_AUTO_TEST_CASE(test_9){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< long long > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 3_z);
}

BOOST_AUTO_TEST_CASE(test_10){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0, 1 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< double, int > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 0_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 0_z);
}

BOOST_AUTO_TEST_CASE(test_11){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0, 1 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< char, bool > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_12){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 0, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< char, long long > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
	static_assert(index.indexes[1_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 3_z);
}

BOOST_AUTO_TEST_CASE(test_13){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 1, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool, float > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 2_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_14){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 1, 2, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< int, long long, char > >());
	static_assert(index);
	static_assert(index.index_count == 3);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 0_z);
	static_assert(index.indexes[2_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[2_c].i, 3_z);
}

BOOST_AUTO_TEST_CASE(test_15){
	constexpr dimension_solver solver{
		list, wrapped_type_ref_c< morph, 1, 2, 1, 0, 0 >};
	auto const index = solver.solve(name,
		type_index::type_id<
			morph< int, long long, int, char, char > >());
	static_assert(index);
	static_assert(index.index_count == 3);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 0_z);
	static_assert(index.indexes[2_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[2_c].i, 3_z);
}

BOOST_AUTO_TEST_CASE(test_16){
	constexpr dimension_solver solver{
		reduce_dimension_list(list, ic< 2, 3 >),
		wrapped_type_ref_c< ambiguous_31, 1, 0, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool, float, long > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 0);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 2_z);
	static_assert(index.indexes[1_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_17){
	constexpr dimension_solver solver{
		reduce_dimension_list(
			reduce_dimension_list(list, ic< 2, 2 >), ic< 0, 2 >),
		wrapped_type_ref_c< ambiguous_32, 1, 0, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool, some_type, some_type > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_18){
	constexpr dimension_solver solver{
		reduce_dimension_list(list, ic< 0, 1 >),
		wrapped_type_ref_c< morph, 1, 2 >};
	auto const index = solver.solve(name,
			type_index::type_id< morph< bool, long > >());
	static_assert(index);
	static_assert(index.index_count == 2);
	static_assert(index.indexes[0_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
	static_assert(index.indexes[1_c].d == 2);
	BOOST_CHECK_EQUAL(index.indexes[1_c].i, 2_z);
}

BOOST_AUTO_TEST_CASE(test_19){
	constexpr dimension_solver solver{
		reduce_dimension_list(
			reduce_dimension_list(list, ic< 0, 1 >), ic< 2, 2 >),
		wrapped_type_ref_c< morph, 1, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool, long > >());
	static_assert(index);
	static_assert(index.index_count == 1);
	static_assert(index.indexes[0_c].d == 1);
	BOOST_CHECK_EQUAL(index.indexes[0_c].i, 1_z);
}

BOOST_AUTO_TEST_CASE(test_20){
	constexpr dimension_solver solver{
		reduce_dimension_list(reduce_dimension_list(reduce_dimension_list(
			list, ic< 0, 0 >), ic< 1, 1 >), ic< 2, 2 >),
		wrapped_type_ref_c< morph, 1, 2 >};
	auto const index = solver.solve(name,
		type_index::type_id< morph< bool, long > >());
	static_assert(!index);
	static_assert(index.index_count == 0);
}
