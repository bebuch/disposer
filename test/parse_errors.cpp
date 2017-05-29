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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>


int success(std::size_t i, std::string const& msg){
	std::cout << std::setw(3) << i << " \033[0;32msuccess:\033[0m "
		<< io_tools::mask_non_print(msg) << "\n";
	return 0;
}

int fail(std::size_t i, std::string const& msg){
	std::cout << std::setw(3) << i << " \033[0;31mfail:\033[0m "
		<< io_tools::mask_non_print(msg) << "\n";
	return 1;
}

std::vector< std::pair< std::string, std::string > > tests{
	// 000
	{
R"file(xparameter_set)file"
	,
"Syntax error at line 1, pos 0: 'xparameter_set', expected keyword line "
"'parameter_set\n' or keyword line 'chain\n'"
	}
	,
	// 001
	{
R"file(parameter_setx)file"
	,
"Syntax error at line 1, pos 13: 'parameter_setx', expected keyword line "
"'parameter_set\n'"
	}
	,
	// 002
	{
R"file(parameter_set)file"
	,
"Syntax error at line 1, pos 13: 'parameter_set', expected keyword line "
"'parameter_set\n'"
	}
	,
	// 003
	{
R"file(parameter_set
)file"
	,
"Syntax error at line 2, pos 0: '', expected at least one parameter set "
"line '\tname\n'"
	}
	,
	// 004
	{
R"file(parameter_set
	name1
	name2
)file"
	,
"Syntax error at line 3, pos 0: '\tname2\n', expected at least one "
"parameter line '\t\tname [= value]\n' with name != 'parameter_set'"
	}
	,
	// 005
	{
R"file(parameter_set
	name1
		parameter_set = a
)file"
	,
"Syntax error at line 3, pos 2: '\t\tparameter_set = a\n', expected a "
"parameter, but a parameter name ('\t\tname [= value]\n') must not be "
"'parameter_set'"
	}
	,
	// 006
	{
R"file(parameter_set
	name1
		param1
			type
)file"
	,
"Syntax error at line 4, pos 7: '\t\t\ttype\n', expected a parameter "
"specialization '\t\t\ttype = value\n'"
	}
	,
	// 007
	{
R"file(parameter_set
	name1
		test=a
	name2
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one "
"parameter line '\t\tname [= value]\n' with name != 'parameter_set'"
	}
	,
	// 008
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
)file"
	,
"Syntax error at line 6, pos 0: '', expected a parameter set line "
"'\tname\n' or a parameter definition ('\t\tname [= value]\n') or a parameter "
"specialization '\t\t\ttype = value\n' or keyword line 'chain\n'"
	}
	,
	// 009
	{
R"file(chain)file"
	,
"Syntax error at line 1, pos 0: 'chain', expected keyword line "
"'parameter_set\n' or keyword line 'chain\n'"
	}
	,
	// 010
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
chain)file"
	,
"Syntax error at line 6, pos 5: 'chain', expected a parameter set line "
"'\tname\n' or a parameter definition ('\t\tname [= value]\n') or a parameter "
"specialization '\t\t\ttype = value\n' or keyword line 'chain\n'"
	}
	,
	// 011
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
chain
)file"
	,
"Syntax error at line 7, pos 0: '', expected at least one chain line "
"'\tname [= id_generator]\n'"
	}
	,
	// 012
	{
R"file(chain
)file"
	,
"Syntax error at line 2, pos 0: '', expected at least one chain line "
"'\tname [= id_generator]\n'"
	}
	,
	// 013
	{
R"file(chain
	chain1 =
)file"
	,
"Syntax error at line 2, pos 9: '\tchain1 =\n', expected a chain line with "
"id_generator '\tname = id_generator\n'"
	}
	,
	// 014
	{
R"file(chain
	chain1
		module1
			parameter)file"
	,
"Syntax error at line 4, pos 12: '\t\t\tparameter', expected keyword line "
"'\t\t\tparameter\n'"
	}
	,
	// 015
	{
R"file(chain
	chain1
		module1
			parameter
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one parameter set "
"reference line '\t\t\t\tparameter_set = name\n', where 'parameter_set' is a "
"keyword and 'name' the name of the referenced parameter set or one parameter "
"'\t\t\t\tname [= value]\n'"
	}
	,
	// 016
	{
R"file(chain
	chain1
		module1
			parameter
				parameter_set =
)file"
	,
"Syntax error at line 5, pos 19: '\t\t\t\tparameter_set =\n', expected a "
"parameter set reference line '\t\t\t\tparameter_set = name\n', where "
"'parameter_set' is a keyword and 'name' the name of the referenced parameter "
"set"
	}
	,
	// 017
	{
R"file(chain
	chain1
		module1
			parameter
				parameter_set = ref1
				name2 =
)file"
	,
"Syntax error at line 6, pos 11: '\t\t\t\tname2 =\n', expected a parameter "
"'\t\t\t\tname [= value]\n' with name != 'parameter_set'"
	}
	,
	// 018
	{
R"file(chain
	chain1
		module1
			parameter
				name2
					type =
)file"
	,
"Syntax error at line 6, pos 11: '\t\t\t\t\ttype =\n', expected a parameter "
"specialization '\t\t\t\t\ttype = value\n'"
	}
	,
	// 019
	{
R"file(chain
	chain1
		module1
			parameter
				name2 = value
				parameter_set = ref1
)file"
	,
"Syntax error at line 6, pos 4: '\t\t\t\tparameter_set = ref1\n', expected "
"another parameter, but a parameter name ('\t\t\t\tname [= value]\n') must not "
"be 'parameter_set'"
	}
	,
	// 020
	{
R"file(chain
	name5
		load
			<-)file"
	,
"Syntax error at line 4, pos 5: '\t\t\t<-', expected keyword line '\t\t\t<-\n'"
	}
	,
	// 021
	{
R"file(chain
	name5
		load
			<-
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one input map "
"'\t\t\t\tinput = {< or &}variable'"
	}
	,
	// 022
	{
R"file(chain
	name5
		load
			<-
				name =
)file"
	,
"Syntax error at line 5, pos 10: '\t\t\t\tname =\n', expected input map "
"'\t\t\t\tinput = {< or &}variable'"
	}
	,
	// 023
	{
R"file(chain
	name5
		load
			->)file"
	,
"Syntax error at line 4, pos 5: '\t\t\t->', expected keyword line '\t\t\t->\n'"
	}
	,
	// 024
	{
R"file(chain
	name5
		load
			->
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one output map "
"'\t\t\t\toutput = >variable'"
	}
	,
	// 025
	{
R"file(chain
	name5
		load
			->
				name
)file"
	,
"Syntax error at line 5, pos 8: '\t\t\t\tname\n', expected output map "
"'\t\t\t\toutput = >variable'"
	}
};

int parse(std::size_t i, std::string content, std::string const& message){
	try{
		std::istringstream file(content);
		disposer::parse(file);
		return fail(i, "No exception");
	}catch(std::exception const& e){
		if(e.what() == message){
			return success(i, e.what());
		}else{
			return fail(i, e.what());
		}
	}catch(...){
		return fail(i, "Unknown exception");
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
