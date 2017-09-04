#include <disposer/core/dimension_referrer.hpp>

#include <iostream>


template < typename ... T > struct morph{};


int main(){
	using namespace disposer;
	using hana::make_tuple;
	using hana::size_c;
	using namespace hana::literals;

	auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >,
			dimension_c< short, unsigned, long, long long >
		};
	using list_t = decltype(list);

	{
		auto c = free_type_c< long >.convert< list_t >;
		static_assert(c.ranges == make_tuple());
		auto type = c.value_type_of(hana::tuple_c< std::size_t >);
		static_assert(type == hana::type_c< long >);
	}

	{
		auto c = type_ref_c< 0 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< double >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< char >);
		auto type_2 = c.value_type_of(hana::tuple_c< std::size_t, 2 >);
		static_assert(type_2 == hana::type_c< float >);
	}

	{
		auto c = type_ref_c< 1 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< int >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< bool >);
	}

	{
		auto c = wrapped_type_ref_c< morph, 0 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< morph< double > >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< morph< char > >);
		auto type_2 = c.value_type_of(hana::tuple_c< std::size_t, 2 >);
		static_assert(type_2 == hana::type_c< morph< float > >);
	}

	{
		auto c = wrapped_type_ref_c< morph, 1 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< morph< int > >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< morph< bool > >);
	}

	{
		auto c = wrapped_type_ref_c< morph, 0, 1 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >),
			make_tuple(size_c< 0 >, size_c< 1 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
		static_assert(type_0 == hana::type_c< morph< double, int > >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
		static_assert(type_1 == hana::type_c< morph< char, int > >);
		auto type_2 = c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
		static_assert(type_2 == hana::type_c< morph< float, int > >);
		auto type_3 = c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
		static_assert(type_3 == hana::type_c< morph< double, bool > >);
		auto type_4 = c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
		static_assert(type_4 == hana::type_c< morph< char, bool > >);
		auto type_5 = c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
		static_assert(type_5 == hana::type_c< morph< float, bool > >);
	}

	{
		auto c = wrapped_type_ref_c< morph, 1, 0, 1 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >),
			make_tuple(size_c< 0 >, size_c< 1 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
		static_assert(type_0 == hana::type_c< morph< int, double, int > >);
		auto type_1= c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
		static_assert(type_1 == hana::type_c< morph< int, char, int > >);
		auto type_2 = c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
		static_assert(type_2 == hana::type_c< morph< int, float, int > >);
		auto type_3 = c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
		static_assert(type_3 == hana::type_c< morph< bool, double, bool > >);
		auto type_4 = c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
		static_assert(type_4 == hana::type_c< morph< bool, char, bool > >);
		auto type_5 = c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
		static_assert(type_5 == hana::type_c< morph< bool, float, bool > >);
	}

	{
		auto c = wrapped_type_ref_c< morph, 2, 0 >.convert< list_t >;
		static_assert(c.ranges == make_tuple(
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >),
			make_tuple(size_c< 0 >, size_c< 1 >, size_c< 2 >, size_c< 3 >)));
		auto type_0 = c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
		static_assert(type_0 == hana::type_c< morph< short, double > >);
		auto type_1 = c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
		static_assert(type_1 == hana::type_c< morph< unsigned, double > >);
		auto type_2 = c.value_type_of(hana::tuple_c< std::size_t, 0, 2 >);
		static_assert(type_2 == hana::type_c< morph< long, double > >);
		auto type_3 = c.value_type_of(hana::tuple_c< std::size_t, 0, 3 >);
		static_assert(type_3 == hana::type_c< morph< long long, double > >);
		auto type_4 = c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
		static_assert(type_4 == hana::type_c< morph< short, char > >);
		auto type_5 = c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
		static_assert(type_5 == hana::type_c< morph< unsigned, char > >);
		auto type_6 = c.value_type_of(hana::tuple_c< std::size_t, 1, 2 >);
		static_assert(type_6 == hana::type_c< morph< long, char > >);
		auto type_7 = c.value_type_of(hana::tuple_c< std::size_t, 1, 3 >);
		static_assert(type_7 == hana::type_c< morph< long long, char > >);
		auto type_8 = c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
		static_assert(type_8 == hana::type_c< morph< short, float > >);
		auto type_9 = c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
		static_assert(type_9 == hana::type_c< morph< unsigned, float > >);
		auto type_10 = c.value_type_of(hana::tuple_c< std::size_t, 2, 2 >);
		static_assert(type_10 == hana::type_c< morph< long, float > >);
		auto type_11 = c.value_type_of(hana::tuple_c< std::size_t, 2, 3 >);
		static_assert(type_11 == hana::type_c< morph< long long, float > >);
	}
}
