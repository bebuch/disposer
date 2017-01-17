//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/log_tag.hpp>
#include <disposer/log.hpp>

#include <boost/type_index.hpp>

#include <iostream>


using disposer::log_tag_base;


// log function types

struct fo1{
	void operator()(log_tag_base& os){
		os << "struct";
	}
};

struct fo2{
	void operator()(log_tag_base& os)const{
		os << "struct const";
	}
};

struct fo3{
	void operator()(log_tag_base& os)volatile{
		os << "struct volatile";
	}
};

struct fo4{
	void operator()(log_tag_base& os)const volatile{
		os << "struct const volatile";
	}
};


void f(log_tag_base& os){
	os << "f";
}

void fp(log_tag_base& os){
	os << "fp";
}

void fr(log_tag_base& os){
	os << "fr";
}

void fpr(log_tag_base& os){
	os << "fpr";
}

void (*p)(log_tag_base& os) = &fp;
void (&r)(log_tag_base& os) = fr;



// body return type

struct x1{
	int operator()(){
		return 1;
	}
};

struct x2{
	int a = 0;
	decltype(auto) operator()(){
		return (a);
	}
};


int main(){
// 	auto has_factory = boost::hana::is_valid([](auto t)->decltype((void)decltype(t)::type::factory()){});
// 	std::cout << (bool)has_factory(boost::hana::type< disposer::log_base >) << std::endl;
// 	std::cout << (bool)has_factory(boost::hana::type< disposer::log_tag_base >) << std::endl;

	std::function< void(log_tag_base&) > stdf_r = [](log_tag_base& os){ os << "stdf_r"; };
	auto lambda_r = [](log_tag_base& os){ os << "lambda_r"; };

	// log function types
	disposer::log([](log_tag_base& os){ os << "lambda"; });
	disposer::log(fo1());
	disposer::log(fo2());
	disposer::log(fo3());
	disposer::log(fo4());
	disposer::log(f);
	disposer::log(&fp);
	disposer::log(p);
	disposer::log(r);
	disposer::log(std::function< void(log_tag_base&) >([](log_tag_base& os){ os << "stdf"; }));
	disposer::log(stdf_r);
	disposer::log(lambda_r);


	// body return type
	std::cout << boost::typeindex::type_id_with_cvr< decltype(disposer::log(f, x1())) >().pretty_name() << " == int" << std::endl;
	std::cout << boost::typeindex::type_id_with_cvr< decltype(disposer::log(f, x2())) >().pretty_name() << " == int&" << std::endl;

	int a = 0;
	decltype(auto) w1 = []{ return 1; }();
	std::cout << boost::typeindex::type_id_with_cvr< decltype(w1) >().pretty_name() << std::endl;
	decltype(auto) w2 = [&a]()->decltype(auto){ return (a); }();
	std::cout << boost::typeindex::type_id_with_cvr< decltype(w2) >().pretty_name() << std::endl;

	disposer::log([](log_tag_base& os){ os << "lambda"; }, []{  });
	decltype(auto) v2 = disposer::log([](log_tag_base& os){ os << "lambda"; }, []{ return 1; });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(v2) >().pretty_name() << std::endl;
	decltype(auto) v3 = disposer::log([](log_tag_base& os){ os << "lambda"; }, [&a]()->decltype(auto){ return (a); });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(v3) >().pretty_name() << std::endl;

	decltype(auto) x1 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{  });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(x1) >().pretty_name() << std::endl;
	decltype(auto) x2 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{ return 1; });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(x2) >().pretty_name() << std::endl;
	decltype(auto) x3 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, [&a]()->decltype(auto){ return (a); });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(x3) >().pretty_name() << std::endl;

	decltype(auto) y1 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{ throw std::runtime_error("test"); });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(y1) >().pretty_name() << std::endl;
	decltype(auto) y2 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{ throw std::logic_error("test"); return 1; });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(y2) >().pretty_name() << std::endl;
	decltype(auto) y3 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, [&a]()->decltype(auto){ throw std::out_of_range("test"); return (a); });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(y3) >().pretty_name() << std::endl;

	decltype(auto) z1 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{ throw ""; });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(z1) >().pretty_name() << std::endl;
	decltype(auto) z2 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, []{ throw 5; return 1; });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(z2) >().pretty_name() << std::endl;
	decltype(auto) z3 = disposer::exception_catching_log([](log_tag_base& os){ os << "exception_catching"; }, [&a]()->decltype(auto){ throw 3.9; return (a); });
	std::cout << boost::typeindex::type_id_with_cvr< decltype(z3) >().pretty_name() << std::endl;
}
