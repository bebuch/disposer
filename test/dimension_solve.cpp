#include <disposer/core/dimension_solve.hpp>

#include <boost/hana/string.hpp>


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

	constexpr auto name = "input_name"_s;

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< double > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 0);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< char > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< float > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 2);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< int > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 1);
		static_assert(index.indexes[0_c].i == 0);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< bool > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 1);
		static_assert(index.indexes[0_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 2 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< short > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 2);
		static_assert(index.indexes[0_c].i == 0);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 2 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< unsigned > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 2);
		static_assert(index.indexes[0_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 2 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< long > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 2);
		static_assert(index.indexes[0_c].i == 2);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 2 >};
		constexpr auto index = solver.solve(name,
			type_index::type_id< morph< long long > >(), make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 2);
		static_assert(index.indexes[0_c].i == 3);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0, 1 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< double, int > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 0);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 0);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0, 1 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< char, bool > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 1);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 0, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< char, long long > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 1);
		static_assert(index.indexes[1_c].d == 2);
		static_assert(index.indexes[1_c].i == 3);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 0 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, float > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 2);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 2, 0 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< int, long long, char > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 3);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 1);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 0);
		static_assert(index.indexes[2_c].d == 2);
		static_assert(index.indexes[2_c].i == 3);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 2, 1, 0, 0 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id<
					morph< int, long long, int, char, char > >(),
				make_list_index< 3 >());
		static_assert(index);
		static_assert(index.index_count == 3);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 1);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 0);
		static_assert(index.indexes[2_c].d == 2);
		static_assert(index.indexes[2_c].i == 3);
	}

// 	{
// 		constexpr dimension_solver solver{
// 			list, wrapped_type_ref_c< ambiguous_31, 1, 0, 2 >};
// 		constexpr auto index = solver.solve(name,
// 				type_index::type_id< morph< bool, float, long > >(),
// 				make_list_index< 3 >());
// 		static_assert(!index);
// 		static_assert(index.index_count == 0);
// 	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< ambiguous_31, 1, 0, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, float, long > >(),
				make_list_index< 3 >(index_component< 2 >{3}));
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 0);
		static_assert(index.indexes[0_c].i == 2);
		static_assert(index.indexes[1_c].d == 1);
		static_assert(index.indexes[1_c].i == 1);
	}

// 	{
// 		constexpr dimension_solver solver{
// 			list, wrapped_type_ref_c< ambiguous_32, 1, 0, 2 >};
// 		constexpr auto index = solver.solve(name,
// 				type_index::type_id< morph< bool, float, long > >(),
// 				make_list_index< 3 >(index_component< 2 >{3}));
// 		static_assert(!index);
// 		static_assert(index.index_count == 0);
// 	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< ambiguous_32, 1, 0, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, some_type, some_type > >(),
				make_list_index< 3 >(
					index_component< 2 >{2}, index_component< 0 >{2}));
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 1);
		static_assert(index.indexes[0_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, long > >(),
				make_list_index< 3 >(index_component< 0 >{0}));
		static_assert(index);
		static_assert(index.index_count == 2);
		static_assert(index.indexes[0_c].d == 1);
		static_assert(index.indexes[0_c].i == 1);
		static_assert(index.indexes[1_c].d == 2);
		static_assert(index.indexes[1_c].i == 2);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, long > >(),
				make_list_index< 3 >(
					index_component< 0 >{0}, index_component< 2 >{2}));
		static_assert(index);
		static_assert(index.index_count == 1);
		static_assert(index.indexes[0_c].d == 1);
		static_assert(index.indexes[0_c].i == 1);
	}

	{
		constexpr dimension_solver solver{
			list, wrapped_type_ref_c< morph, 1, 2 >};
		constexpr auto index = solver.solve(name,
				type_index::type_id< morph< bool, long > >(),
				make_list_index< 3 >(
					index_component< 0 >{0},
					index_component< 1 >{0},
					index_component< 2 >{0}));
		static_assert(!index);
		static_assert(index.index_count == 0);
	}
}
