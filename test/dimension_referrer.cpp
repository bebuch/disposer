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

	{
		auto free_type = free_type_c< long >;
		auto type = free_type.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t >);
		static_assert(type == hana::type_c< long >);
	}

	{
		auto list = type_ref_c< 0 >;
		auto type_0 = list.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< double >);
		auto type_1 = list.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< char >);
		auto type_2 = list.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2 >);
		static_assert(type_2 == hana::type_c< float >);
	}

	{
		auto list = type_ref_c< 1 >;
		auto type_0 = list.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< int >);
		auto type_1 = list.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< bool >);
	}

	{
		auto wrap = wrapped_type_ref_c< morph, 0 >;
		auto type_0 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< morph< double > >);
		auto type_1 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< morph< char > >);
		auto type_2 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2 >);
		static_assert(type_2 == hana::type_c< morph< float > >);
	}

	{
		auto wrap = wrapped_type_ref_c< morph, 1 >;
		auto type_0 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0 >);
		static_assert(type_0 == hana::type_c< morph< int > >);
		auto type_1 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1 >);
		static_assert(type_1 == hana::type_c< morph< bool > >);
	}

	{
		auto wrap = wrapped_type_ref_c< morph, 0, 1 >;
		auto type_0 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
		static_assert(type_0 == hana::type_c< morph< double, int > >);
		auto type_1 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
		static_assert(type_1 == hana::type_c< morph< char, int > >);
		auto type_2 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
		static_assert(type_2 == hana::type_c< morph< float, int > >);
		auto type_3 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
		static_assert(type_3 == hana::type_c< morph< double, bool > >);
		auto type_4 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
		static_assert(type_4 == hana::type_c< morph< char, bool > >);
		auto type_5 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
		static_assert(type_5 == hana::type_c< morph< float, bool > >);
	}

	{
		auto wrap = wrapped_type_ref_c< morph, 1, 0, 1 >;
		auto type_0 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0, 0 >);
		static_assert(type_0 == hana::type_c< morph< int, double, int > >);
		auto type_1= wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1, 0 >);
		static_assert(type_1 == hana::type_c< morph< int, char, int > >);
		auto type_2 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2, 0 >);
		static_assert(type_2 == hana::type_c< morph< int, float, int > >);
		auto type_3 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 0, 1 >);
		static_assert(type_3 == hana::type_c< morph< bool, double, bool > >);
		auto type_4 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 1, 1 >);
		static_assert(type_4 == hana::type_c< morph< bool, char, bool > >);
		auto type_5 = wrap.convert< list_t >
			.value_type_of(hana::tuple_c< std::size_t, 2, 1 >);
		static_assert(type_5 == hana::type_c< morph< bool, float, bool > >);
	}
}
