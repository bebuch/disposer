#include <disposer/core/dimension_referrer.hpp>


template < typename ... T > struct morph{};


int main(){
	using namespace disposer;

	// wrap_type_ref_in_t
	auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >
		};
	using list_t = decltype(list);

	auto list_f = free_type_c< long >;
	auto type_f_00 = list_f.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t >);
	static_assert(type_f_00 == hana::type_c< long >);

	auto list_0 = type_ref_c< 0 >;
	auto type_0_00 = list_0.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0 >);
	static_assert(type_0_00 == hana::type_c< double >);
	auto type_0_10 = list_0.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1 >);
	static_assert(type_0_10 == hana::type_c< char >);
	auto type_0_20 = list_0.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2 >);
	static_assert(type_0_20 == hana::type_c< float >);

	auto list_1 = type_ref_c< 1 >;
	auto type_1_20 = list_1.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0 >);
	static_assert(type_1_20 == hana::type_c< int >);
	auto type_1_21 = list_1.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1 >);
	static_assert(type_1_21 == hana::type_c< bool >);

	auto wrap_10 = wrapped_type_ref_c< morph, 0 >;
	auto type_10_00 = wrap_10.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0 >);
	static_assert(type_10_00 == hana::type_c< morph< double > >);
	auto type_10_10 = wrap_10.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1 >);
	static_assert(type_10_10 == hana::type_c< morph< char > >);
	auto type_10_20 = wrap_10.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2 >);
	static_assert(type_10_20 == hana::type_c< morph< float > >);

	auto wrap_11 = wrapped_type_ref_c< morph, 1 >;
	auto type_11_00 = wrap_11.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0 >);
	static_assert(type_11_00 == hana::type_c< morph< int > >);
	auto type_11_01 = wrap_11.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1 >);
	static_assert(type_11_01 == hana::type_c< morph< bool > >);

	auto wrap_2 = wrapped_type_ref_c< morph, 0, 1 >;
	auto type_2_00 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
	static_assert(type_2_00 == hana::type_c< morph< double, int > >);
	auto type_2_10 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
	static_assert(type_2_10 == hana::type_c< morph< char, int > >);
	auto type_2_20 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
	static_assert(type_2_20 == hana::type_c< morph< float, int > >);
	auto type_2_01 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
	static_assert(type_2_01 == hana::type_c< morph< double, bool > >);
	auto type_2_11 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
	static_assert(type_2_11 == hana::type_c< morph< char, bool > >);
	auto type_2_21 = wrap_2.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
	static_assert(type_2_21 == hana::type_c< morph< float, bool > >);

	auto wrap_r = wrapped_type_ref_c< morph, 1, 0, 1 >;
	auto type_r_00 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
	static_assert(type_r_00 == hana::type_c< morph< int, double, int > >);
	auto type_r_10 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
	static_assert(type_r_10 == hana::type_c< morph< int, char, int > >);
	auto type_r_20 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
	static_assert(type_r_20 == hana::type_c< morph< int, float, int > >);
	auto type_r_01 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
	static_assert(type_r_01 == hana::type_c< morph< bool, double, bool > >);
	auto type_r_11 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
	static_assert(type_r_11 == hana::type_c< morph< bool, char, bool > >);
	auto type_r_21 = wrap_r.convert< list_t >
		.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
	static_assert(type_r_21 == hana::type_c< morph< bool, float, bool > >);
}
