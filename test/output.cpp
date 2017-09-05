#include <disposer/core/make_output.hpp>

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


namespace hana = boost::hana;

using namespace disposer;

using namespace hana::literals;
using namespace disposer::literals;
using namespace std::literals::string_view_literals;


template < typename Data, typename T >
int check(std::size_t i, Data const& data, hana::basic_type< T >){
	if(std::holds_alternative< output_construct_data< decltype("v"_out), T > >
		(hana::second(data[0_c]))
	){
		return success(i);
	}else{
		return fail(i);
	}
}

template < std::size_t D >
using ic = index_component< D >;

template < typename ... T > struct morph{};


int main(){
	using hana::type_c;

	std::size_t ct = 0;
	std::size_t ec = 0;

	constexpr auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >,
			dimension_c< short, unsigned, long, long long >
		};

	auto const m = [list](auto const& maker, auto const& ... ics){
			return make_construct_data(maker, list, module_make_data{},
				make_list_index< decltype(hana::size(list.dimensions))::value >
					(ics ...), hana::make_tuple());
		};

	try{
		{
			constexpr auto maker = make("v"_out, type_ref_c< 0 >);
			ec = check(ct++, m(maker, ic< 0 >{0}), type_c< double >);
			ec = check(ct++, m(maker, ic< 0 >{1}), type_c< char >);
			ec = check(ct++, m(maker, ic< 0 >{2}), type_c< float >);
		}

		{
			constexpr auto maker = make("v"_out, type_ref_c< 1 >);
			ec = check(ct++, m(maker, ic< 1 >{0}), type_c< int >);
			ec = check(ct++, m(maker, ic< 1 >{1}), type_c< bool >);
		}

		{
			constexpr auto maker = make("v"_out, type_ref_c< 2 >);
			ec = check(ct++, m(maker, ic< 2 >{0}), type_c< short >);
			ec = check(ct++, m(maker, ic< 2 >{1}), type_c< unsigned >);
			ec = check(ct++, m(maker, ic< 2 >{2}), type_c< long >);
			ec = check(ct++, m(maker, ic< 2 >{3}), type_c< long long >);
		}

		{
			constexpr auto maker = make("v"_out,
				wrapped_type_ref_c< morph, 0, 1 >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 1 >{0}),
				type_c< morph< double, int > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 1 >{0}),
				type_c< morph< char, int > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 1 >{0}),
				type_c< morph< float, int > >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 1 >{1}),
				type_c< morph< double, bool > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 1 >{1}),
				type_c< morph< char, bool > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 1 >{1}),
				type_c< morph< float, bool > >);
		}

		{
			constexpr auto maker = make("v"_out,
				wrapped_type_ref_c< morph, 2, 0 >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 2 >{0}),
				type_c< morph< short, double > >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 2 >{1}),
				type_c< morph< unsigned, double > >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 2 >{2}),
				type_c< morph< long, double > >);
			ec = check(ct++, m(maker, ic< 0 >{0}, ic< 2 >{3}),
				type_c< morph< long long, double > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 2 >{0}),
				type_c< morph< short, char > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 2 >{1}),
				type_c< morph< unsigned, char > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 2 >{2}),
				type_c< morph< long, char > >);
			ec = check(ct++, m(maker, ic< 0 >{1}, ic< 2 >{3}),
				type_c< morph< long long, char > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 2 >{0}),
				type_c< morph< short, float > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 2 >{1}),
				type_c< morph< unsigned, float > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 2 >{2}),
				type_c< morph< long, float > >);
			ec = check(ct++, m(maker, ic< 0 >{2}, ic< 2 >{3}),
				type_c< morph< long long, float > >);
		}

		if(ec == 0){
			std::cout << "\033[0;32mSUCCESS\033[0m\n";
		}else{
			std::cout << "\033[0;31mFAILS:\033[0m " << ec << '\n';
		}
	}catch(std::exception const& e){
		std::cerr << "Unexpected exception: " << e.what() << '\n';
	}catch(...){
		std::cerr << "Unexpected unknown exception\n";
	}

	return ec;
}
