//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/parse.hpp>

#include <io_tools/mask_non_print.hpp>

#include <boost/optional/optional_io.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace disposer{ namespace types{ namespace parse{

	std::ostream& operator<<(std::ostream& os, std::string const& v){
		os << '"';
		std::operator<<(os, v);
		os << '"';
		return os;
	}

	template < typename T >
	std::ostream& operator<<(std::ostream& os, std::vector< T > const& v){
		os << '{';
		auto iter = v.cbegin();
		if(iter != v.cend()) os << *iter++;
		while(iter != v.cend()) os << ',' << *iter++;
		os << "}";
		return os;
	}


	std::ostream& operator<<(std::ostream& os, parameter const& v){
		return os << "{" << v.key << "," << v.value << "}";
	}

	std::ostream& operator<<(std::ostream& os, parameter_set const& v){
		return os << "{" << v.name << "," << v.parameters << "}";
	}

	std::ostream& operator<<(std::ostream& os, module const& v){
		return os << "{" << v.name << "," << v.type_name << ","
			<< v.parameter_sets << "," << v.parameters << "}";
	}

	std::ostream& operator<<(std::ostream& os, io const& v){
		return os << "{" << v.name << "," << v.variable << "}";
	}

	std::ostream& operator<<(std::ostream& os, chain_module const& v){
		return os << "{" << v.name << "," << v.inputs << ","
			<< v.outputs << "}";
	}

	std::ostream& operator<<(std::ostream& os, chain const& v){
		return os << "{" << v.name << "," << v.group << ","
			<< v.id_generator << "," << v.modules << "}";
	}

	std::ostream& operator<<(std::ostream& os, config const& v){
		return os << "{" << v.sets << "," << v.modules << ","
			<< v.chains << "}";
	}

	bool operator==(
		parameter const& l,
		parameter const& r
	){
		return l.key == r.key
			&& l.value == r.value;
	}

	bool operator==(
		parameter_set const& l,
		parameter_set const& r
	){
		return l.name == r.name
			&& l.parameters == r.parameters;
	}

	bool operator==(
		module const& l,
		module const& r
	){
		return l.name == r.name
			&& l.type_name == r.type_name
			&& l.parameter_sets == r.parameter_sets
			&& l.parameters == r.parameters;
	}

	bool operator==(
		io const& l,
		io const& r
	){
		return l.name == r.name
			&& l.variable == r.variable;
	}

	bool operator==(
		chain_module const& l,
		chain_module const& r
	){
		return l.name == r.name
			&& l.inputs == r.inputs
			&& l.outputs == r.outputs;
	}

	bool operator==(
		chain const& l,
		chain const& r
	){
		return l.name == r.name
			&& l.group == r.group
			&& l.id_generator == r.id_generator
			&& l.modules == r.modules;
	}

	bool operator==(
		config const& l,
		config const& r
	){
		return l.sets == r.sets
			&& l.modules == r.modules
			&& l.chains == r.chains;
	}

} } }

using disposer::types::parse::config;

int success(std::size_t i){
	std::cout << std::setw(3) << i << " \033[0;32msuccess:\033[0m";;
	return 0;
}

int fail2(std::size_t i, config const& ref, config const& test){
	std::ostringstream os1;
	std::ostringstream os2;
	os1 << ref;
	os2 << test;
	std::cout << std::setw(3) << i << " \033[0;31mfail:\033[0m "
		<< "\n" << io_tools::mask_non_print(os1.str())
		<< "\n" << io_tools::mask_non_print(os2.str()) << "\n";
	return 1;
}

int fail1(std::size_t i, std::string const& msg){
	std::cout << std::setw(3) << i << " \033[0;31mfail:\033[0m "
		<< io_tools::mask_non_print(msg) << "\n";
	return 1;
}


std::vector< std::pair< std::string, config > > tests{
	{
R"file(parameter_set
	ps1
		param1 = v1
module
	mod1 = dmod1
		parameter_set = ps1
		param2 = v2
chain
	chain1
		mod1
			->
				out = x1
)file"
	,
		config{
			{
				{
					"ps1",
					{
						{"param1", "v1"}
					}
				}
			},
			{
				{
					"mod1",
					"dmod1",
					{
						{"ps1"}
					}, {
						{"param2", "v2"}
					}
				}
			},
			{
				{
					"chain1",
					{},
					{},
					{
						{
							"mod1",
							{},
							{
								{"out", "x1"}
							}
						}
					}
				}
			}
		}
	}
};

int parse(std::size_t i, std::string content, config const& conf){
	try{
		std::istringstream file(content);
		auto result = disposer::parse(file);
		if(result == conf){
			return success(i);
		}else{
			return fail2(i, conf, result);
		}
	}catch(std::exception const& e){
		return fail1(i, e.what());
	}catch(...){
		return fail1(i, "Unknown exception");
	}
}

int main(){
	std::cout << std::setfill('0');
	std::size_t i = 0;
	std::size_t r = 0;
	for(auto const& v: tests){
		r += parse(i++, v.first, v.second);
	}

	if(r == 0){
		std::cout << "\033[0;32mSUCCESS\033[0m\n";
	}else{
		std::cout << "\033[0;31mFAILS:\033[0m " << r << '\n';
	}
}
