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


using type_index = boost::typeindex::ctti_type_index;


template < typename T >
int check(std::size_t i, T const& test, T const& ref){
	if(test == ref){
		return success(i);
	}else{
		return fail(i);
	}
}


namespace hana = boost::hana;

using namespace hana::literals;
using namespace disposer::literals;
using namespace std::literals::string_view_literals;

using disposer::make;


int main(){
	using namespace disposer;
	using hana::type_c;

// 	std::size_t ct = 0;
	std::size_t error_count = 0;

	auto list = dimension_list{
			dimension_c< double, char, float >,
			dimension_c< int, bool >,
			dimension_c< short, unsigned, long, long long >
		};

	auto const data = module_make_data{};

	try{
		{
			constexpr auto maker = make("v"_out, type_ref_c< 0 >);
			auto construct_data = make_construct_data(maker, list, data,
				make_list_index< 3 >(index_component< 0 >{0}),
				hana::make_tuple());
			(void)construct_data;
		}

		if(error_count == 0){
			std::cout << "\033[0;32mSUCCESS\033[0m\n";
		}else{
			std::cout << "\033[0;31mFAILS:\033[0m " << error_count << '\n';
		}
	}catch(std::exception const& e){
		std::cerr << "Unexpected exception: " << e.what() << '\n';
	}catch(...){
		std::cerr << "Unexpected unknown exception\n";
	}

	return error_count;
}
