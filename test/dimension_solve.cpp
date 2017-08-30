#include <disposer/core/dimension_solve.hpp>


template < typename ... T > struct morph{};

template < typename T1, typename T2, typename >
using ambiguous_31 = morph< T1, T2, long >;


struct some_type{};

template < typename T1, typename, typename >
using ambiguous_32 = morph< T1, some_type, some_type >;


int main(){
	using namespace disposer;
	using namespace hana::literals;

	// wrap_type_ref_in_t
	auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >,
			dimension_c< short, unsigned, long, long long >
		};

	using dims = decltype(list);

	constexpr auto index_10_0 = dimension_solver< dims, morph, 0 >::solve(
		type_index::type_id< morph< double > >(), hana::make_tuple());
	static_assert(index_10_0);
	static_assert(index_10_0.index_count == 1);
	static_assert(index_10_0.indexes[0_c].d == 0);
	static_assert(index_10_0.indexes[0_c].i == 0);

	constexpr auto index_10_1 = dimension_solver< dims, morph, 0 >::solve(
		type_index::type_id< morph< char > >(), hana::make_tuple());
	static_assert(index_10_1);
	static_assert(index_10_1.index_count == 1);
	static_assert(index_10_1.indexes[0_c].d == 0);
	static_assert(index_10_1.indexes[0_c].i == 1);

	constexpr auto index_10_2 = dimension_solver< dims, morph, 0 >::solve(
		type_index::type_id< morph< float > >(), hana::make_tuple());
	static_assert(index_10_2);
	static_assert(index_10_2.index_count == 1);
	static_assert(index_10_2.indexes[0_c].d == 0);
	static_assert(index_10_2.indexes[0_c].i == 2);

	constexpr auto index_11_0 = dimension_solver< dims, morph, 1 >::solve(
		type_index::type_id< morph< int > >(), hana::make_tuple());
	static_assert(index_11_0);
	static_assert(index_11_0.index_count == 1);
	static_assert(index_11_0.indexes[0_c].d == 1);
	static_assert(index_11_0.indexes[0_c].i == 0);

	constexpr auto index_11_1 = dimension_solver< dims, morph, 1 >::solve(
		type_index::type_id< morph< bool > >(), hana::make_tuple());
	static_assert(index_11_1);
	static_assert(index_11_1.index_count == 1);
	static_assert(index_11_1.indexes[0_c].d == 1);
	static_assert(index_11_1.indexes[0_c].i == 1);

	constexpr auto index_12_0 = dimension_solver< dims, morph, 2 >::solve(
		type_index::type_id< morph< short > >(), hana::make_tuple());
	static_assert(index_12_0);
	static_assert(index_12_0.index_count == 1);
	static_assert(index_12_0.indexes[0_c].d == 2);
	static_assert(index_12_0.indexes[0_c].i == 0);

	constexpr auto index_12_1 = dimension_solver< dims, morph, 2 >::solve(
		type_index::type_id< morph< unsigned > >(), hana::make_tuple());
	static_assert(index_12_1);
	static_assert(index_12_1.index_count == 1);
	static_assert(index_12_1.indexes[0_c].d == 2);
	static_assert(index_12_1.indexes[0_c].i == 1);

	constexpr auto index_12_2 = dimension_solver< dims, morph, 2 >::solve(
		type_index::type_id< morph< long > >(), hana::make_tuple());
	static_assert(index_12_2);
	static_assert(index_12_2.index_count == 1);
	static_assert(index_12_2.indexes[0_c].d == 2);
	static_assert(index_12_2.indexes[0_c].i == 2);

	constexpr auto index_12_3 = dimension_solver< dims, morph, 2 >::solve(
		type_index::type_id< morph< long long > >(), hana::make_tuple());
	static_assert(index_12_3);
	static_assert(index_12_3.index_count == 1);
	static_assert(index_12_3.indexes[0_c].d == 2);
	static_assert(index_12_3.indexes[0_c].i == 3);


	constexpr auto index_201_00 =
		dimension_solver< dims, morph, 0, 1 >::solve(
			type_index::type_id< morph< double, int > >(),
			hana::make_tuple());
	static_assert(index_201_00);
	static_assert(index_201_00.index_count == 2);
	static_assert(index_201_00.indexes[0_c].d == 0);
	static_assert(index_201_00.indexes[0_c].i == 0);
	static_assert(index_201_00.indexes[1_c].d == 1);
	static_assert(index_201_00.indexes[1_c].i == 0);

	constexpr auto index_201_11 =
		dimension_solver< dims, morph, 0, 1 >::solve(
			type_index::type_id< morph< char, bool > >(),
			hana::make_tuple());
	static_assert(index_201_11);
	static_assert(index_201_11.index_count == 2);
	static_assert(index_201_11.indexes[0_c].d == 0);
	static_assert(index_201_11.indexes[0_c].i == 1);
	static_assert(index_201_11.indexes[1_c].d == 1);
	static_assert(index_201_11.indexes[1_c].i == 1);

	constexpr auto index_202_13 =
		dimension_solver< dims, morph, 0, 2 >::solve(
			type_index::type_id< morph< char, long long > >(),
			hana::make_tuple());
	static_assert(index_202_13);
	static_assert(index_202_13.index_count == 2);
	static_assert(index_202_13.indexes[0_c].d == 0);
	static_assert(index_202_13.indexes[0_c].i == 1);
	static_assert(index_202_13.indexes[1_c].d == 2);
	static_assert(index_202_13.indexes[1_c].i == 3);


	constexpr auto index_210_12 =
		dimension_solver< dims, morph, 1, 0 >::solve(
			type_index::type_id< morph< bool, float > >(),
			hana::make_tuple());
	static_assert(index_210_12);
	static_assert(index_210_12.index_count == 2);
	static_assert(index_210_12.indexes[0_c].d == 0);
	static_assert(index_210_12.indexes[0_c].i == 2);
	static_assert(index_210_12.indexes[1_c].d == 1);
	static_assert(index_210_12.indexes[1_c].i == 1);

	constexpr auto index_3120_103 =
		dimension_solver< dims, morph, 1, 2, 0 >::solve(
			type_index::type_id< morph< int, long long, char > >(),
			hana::make_tuple());
	static_assert(index_3120_103);
	static_assert(index_3120_103.index_count == 3);
	static_assert(index_3120_103.indexes[0_c].d == 0);
	static_assert(index_3120_103.indexes[0_c].i == 1);
	static_assert(index_3120_103.indexes[1_c].d == 1);
	static_assert(index_3120_103.indexes[1_c].i == 0);
	static_assert(index_3120_103.indexes[2_c].d == 2);
	static_assert(index_3120_103.indexes[2_c].i == 3);

	constexpr auto index_312100_10133 =
		dimension_solver< dims, morph, 1, 2, 1, 0, 0 >::solve(
			type_index::type_id<
				morph< int, long long, int, char, char > >(),
			hana::make_tuple());
	static_assert(index_312100_10133);
	static_assert(index_312100_10133.index_count == 3);
	static_assert(index_312100_10133.indexes[0_c].d == 0);
	static_assert(index_312100_10133.indexes[0_c].i == 1);
	static_assert(index_312100_10133.indexes[1_c].d == 1);
	static_assert(index_312100_10133.indexes[1_c].i == 0);
	static_assert(index_312100_10133.indexes[2_c].d == 2);
	static_assert(index_312100_10133.indexes[2_c].i == 3);


	constexpr auto index_210_12_a31 =
		dimension_solver< dims, ambiguous_31, 1, 0, 2 >::solve(
			type_index::type_id< morph< bool, float, long > >(),
			hana::make_tuple());
	static_assert(!index_210_12_a31);
	static_assert(index_210_12_a31.index_count == 0);


	constexpr auto index_210_12_k2 =
		dimension_solver< dims, ambiguous_31, 1, 0, 2 >::solve(
			type_index::type_id< morph< bool, float, long > >(),
			hana::make_tuple(index_component< 2 >{3}));
	static_assert(index_210_12_k2);
	static_assert(index_210_12_k2.index_count == 2);
	static_assert(index_210_12_k2.indexes[0_c].d == 0);
	static_assert(index_210_12_k2.indexes[0_c].i == 2);
	static_assert(index_210_12_k2.indexes[1_c].d == 1);
	static_assert(index_210_12_k2.indexes[1_c].i == 1);


	constexpr auto index_210_12_a32 =
		dimension_solver< dims, ambiguous_32, 1, 0, 2 >::solve(
			type_index::type_id< morph< bool, float, long > >(),
			hana::make_tuple(index_component< 2 >{3}));
	static_assert(!index_210_12_a32);
	static_assert(index_210_12_a32.index_count == 0);


	constexpr auto index_210_12_k12 =
		dimension_solver< dims, ambiguous_32, 1, 0, 2 >::solve(
			type_index::type_id< morph< bool, some_type, some_type > >(),
			hana::make_tuple(
				index_component< 2 >{2}, index_component< 0 >{2}));
	static_assert(index_210_12_k12);
	static_assert(index_210_12_k12.index_count == 1);
	static_assert(index_210_12_k12.indexes[0_c].d == 1);
	static_assert(index_210_12_k12.indexes[0_c].i == 1);
}
