#include <disposer/core/dimension_referrer.hpp>

#include <iostream>
#include <iomanip>


int success(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;32msuccess\033[0m\n";
	return 0;
}

int fail(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;31mfail\033[0m;\n";
	return 1;
}


template < typename T >
int check(std::size_t i, T const& test, T const& ref){
	if(test == ref){
		return success(i);
	}else{
		return fail(i);
	}
}


using namespace disposer;
using hana::make_tuple;
using hana::type_c;
using hana::tuple_c;
using hana::tuple_t;
using namespace hana::literals;

auto list = dimension_list{
		dimension_c< double, char, float >,
		dimension_c< int, bool >,
		dimension_c< short, unsigned, long, long long >
	};
using list_t = decltype(list);

template < std::size_t D >
using ic = index_component< D >;

template < std::size_t ... D >
auto index(ic< D > const& ... i){
	return packed_index{
		partial_deduced_list_index{
			std::make_index_sequence< hana::size(list.dimensions).value >(),
			undeduced_list_index_c< hana::size(list.dimensions).value >,
			solved_dimensions{i ...}
		}, hana::tuple_c< std::size_t, D ... >};
}

template < typename ... T > struct morph{};


std::size_t ct = 0;
std::size_t ec = 0;

int exec()try{

	{
		auto c = free_type_c< long >.convert< list_t >;
		static_assert(c.indexes == make_tuple(make_tuple()));

		static_assert(c.value_type_of(hana::tuple_c< std::size_t >)
			== type_c< long >);

		static_assert(hana::to_set(c.types) == hana::to_set(tuple_t< long >));

		ec = check(ct++, c.to_type_index(
			index()), type_index::type_id< long >());
	}

	{
		auto c = type_ref_c< 0 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0 >,
			tuple_c< std::size_t, 1 >,
			tuple_c< std::size_t, 2 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0 >)
			== type_c< double >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1 >)
			== type_c< char >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2 >)
			== type_c< float >);

		static_assert(hana::to_set(c.types) == hana::to_set(
			tuple_t< double, char, float >));

		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0})), type_index::type_id< double >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1})), type_index::type_id< char >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2})), type_index::type_id< float >());
	}

	{
		auto c = type_ref_c< 1 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0 >,
			tuple_c< std::size_t, 1 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0 >)
			== type_c< int >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1 >)
			== type_c< bool >);

		static_assert(hana::to_set(c.types) == hana::to_set(
			tuple_t< int, bool >));

		ec = check(ct++, c.to_type_index(
			index(ic< 1 >{0})), type_index::type_id< int >());
		ec = check(ct++, c.to_type_index(
			index(ic< 1 >{1})), type_index::type_id< bool >());
	}

	{
		auto c = wrapped_type_ref_c< morph, 0 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0 >,
			tuple_c< std::size_t, 1 >,
			tuple_c< std::size_t, 2 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0 >)
			== type_c< morph< double > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1 >)
			== type_c< morph< char > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2 >)
			== type_c< morph< float > >);

		static_assert(hana::to_set(c.types) == hana::to_set(
			tuple_t< morph< double >, morph< char >, morph< float > >));

		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0})), type_index::type_id< morph< double > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1})), type_index::type_id< morph< char > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2})), type_index::type_id< morph< float > >());
	}

	{
		auto c = wrapped_type_ref_c< morph, 1 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0 >,
			tuple_c< std::size_t, 1 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0 >)
			== type_c< morph< int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1 >)
			== type_c< morph< bool > >);

		static_assert(hana::to_set(c.types) == hana::to_set(
			tuple_t< morph< int >, morph< bool > >));

		ec = check(ct++, c.to_type_index(
			index(ic< 1 >{0})), type_index::type_id< morph< int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 1 >{1})), type_index::type_id< morph< bool > >());
	}

	{
		auto c = wrapped_type_ref_c< morph, 0, 1 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0, 0 >,
			tuple_c< std::size_t, 0, 1 >,
			tuple_c< std::size_t, 1, 0 >,
			tuple_c< std::size_t, 1, 1 >,
			tuple_c< std::size_t, 2, 0 >,
			tuple_c< std::size_t, 2, 1 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >)
			== type_c< morph< double, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >)
			== type_c< morph< char, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >)
			== type_c< morph< float, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >)
			== type_c< morph< double, bool > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >)
			== type_c< morph< char, bool > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >)
			== type_c< morph< float, bool > >);

		static_assert(hana::to_set(c.types) == hana::to_set(tuple_t<
				morph< double, int >,
				morph< char, int >,
				morph< float, int >,
				morph< double, bool >,
				morph< char, bool >,
				morph< float, bool >
			>));

		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 1 >{0})),
			type_index::type_id< morph< double, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 1 >{0})),
			type_index::type_id< morph< char, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 1 >{0})),
			type_index::type_id< morph< float, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 1 >{1})),
			type_index::type_id< morph< double, bool > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 1 >{1})),
			type_index::type_id< morph< char, bool > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 1 >{1})),
			type_index::type_id< morph< float, bool > >());
	}

	{
		auto c = wrapped_type_ref_c< morph, 1, 0, 1 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0, 0 >,
			tuple_c< std::size_t, 0, 1 >,
			tuple_c< std::size_t, 1, 0 >,
			tuple_c< std::size_t, 1, 1 >,
			tuple_c< std::size_t, 2, 0 >,
			tuple_c< std::size_t, 2, 1 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >)
			== type_c< morph< int, double, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >)
			== type_c< morph< int, char, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >)
			== type_c< morph< int, float, int > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >)
			== type_c< morph< bool, double, bool > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >)
			== type_c< morph< bool, char, bool > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >)
			== type_c< morph< bool, float, bool > >);

		static_assert(hana::to_set(c.types) == hana::to_set(tuple_t<
				morph< int, double, int >,
				morph< int, char, int >,
				morph< int, float, int >,
				morph< bool, double, bool >,
				morph< bool, char, bool >,
				morph< bool, float, bool >
			>));

		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 1 >{0})),
			type_index::type_id< morph< int, double, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 1 >{0})),
			type_index::type_id< morph< int, char, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 1 >{0})),
			type_index::type_id< morph< int, float, int > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 1 >{1})),
			type_index::type_id< morph< bool, double, bool > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 1 >{1})),
			type_index::type_id< morph< bool, char, bool > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 1 >{1})),
			type_index::type_id< morph< bool, float, bool > >());
	}

	{
		auto c = wrapped_type_ref_c< morph, 2, 0 >.convert< list_t >;
		static_assert(c.indexes == make_tuple(
			tuple_c< std::size_t, 0, 0 >,
			tuple_c< std::size_t, 0, 1 >,
			tuple_c< std::size_t, 0, 2 >,
			tuple_c< std::size_t, 0, 3 >,
			tuple_c< std::size_t, 1, 0 >,
			tuple_c< std::size_t, 1, 1 >,
			tuple_c< std::size_t, 1, 2 >,
			tuple_c< std::size_t, 1, 3 >,
			tuple_c< std::size_t, 2, 0 >,
			tuple_c< std::size_t, 2, 1 >,
			tuple_c< std::size_t, 2, 2 >,
			tuple_c< std::size_t, 2, 3 >));
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 0 >)
			== type_c< morph< short, double > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 1 >)
			== type_c< morph< unsigned, double > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 2 >)
			== type_c< morph< long, double > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 0, 3 >)
			== type_c< morph< long long, double > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 0 >)
			== type_c< morph< short, char > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 1 >)
			== type_c< morph< unsigned, char > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 2 >)
			== type_c< morph< long, char > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 1, 3 >)
			== type_c< morph< long long, char > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 0 >)
			== type_c< morph< short, float > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 1 >)
			== type_c< morph< unsigned, float > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 2 >)
			== type_c< morph< long, float > >);
		static_assert(c.value_type_of(hana::tuple_c< std::size_t, 2, 3 >)
			== type_c< morph< long long, float > >);

		static_assert(hana::to_set(c.types) == hana::to_set(tuple_t<
				morph< short, double >,
				morph< unsigned, double >,
				morph< long, double >,
				morph< long long, double >,
				morph< short, char >,
				morph< unsigned, char >,
				morph< long, char >,
				morph< long long, char >,
				morph< short, float >,
				morph< unsigned, float >,
				morph< long, float >,
				morph< long long, float >
			>));

		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 2 >{0})),
			type_index::type_id< morph< short, double > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 2 >{0})),
			type_index::type_id< morph< short, char > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 2 >{0})),
			type_index::type_id< morph< short, float > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 2 >{1})),
			type_index::type_id< morph< unsigned, double > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 2 >{1})),
			type_index::type_id< morph< unsigned, char > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 2 >{1})),
			type_index::type_id< morph< unsigned, float > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 2 >{2})),
			type_index::type_id< morph< long, double > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 2 >{2})),
			type_index::type_id< morph< long, char > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 2 >{2})),
			type_index::type_id< morph< long, float > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{0}, ic< 2 >{3})),
			type_index::type_id< morph< long long, double > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{1}, ic< 2 >{3})),
			type_index::type_id< morph< long long, char > >());
		ec = check(ct++, c.to_type_index(
			index(ic< 0 >{2}, ic< 2 >{3})),
			type_index::type_id< morph< long long, float > >());
	}

	if(ec == 0){
		std::cout << "\033[0;32mSUCCESS\033[0m\n";
	}else{
		std::cout << "\033[0;31mFAILS:\033[0m " << ec << '\n';
	}

	return ec;
}catch(std::exception const& e){
	std::cerr << "Unexpected exception: " << e.what() << '\n';
	return ec;
}catch(...){
	std::cerr << "Unexpected unknown exception\n";
	return ec;
}

int main(){
	return exec();
}
