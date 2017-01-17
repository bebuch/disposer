//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/parse.hpp>
#include <disposer/mask_non_print.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>


int success(std::size_t i, std::string const& msg){
	std::cout << std::setw(3) << i << " \033[0;32msuccess:\033[0m "
		<< disposer::mask_non_print(msg) << "\n";
	return 0;
}

int fail(std::size_t i, std::string const& msg){
	std::cout << std::setw(3) << i << " \033[0;31mfail:\033[0m "
		<< disposer::mask_non_print(msg) << "\n";
	return 1;
}

std::vector< std::pair< std::string, std::string > > tests{
	{
R"file(xparameter_set)file"
	,
"Syntax error at line 1, pos 0: 'xparameter_set', expected keyword line "
"'parameter_set\n' or keyword line 'module\n'"
	}
	,
	{
R"file(parameter_setx)file"
	,
"Syntax error at line 1, pos 13: 'parameter_setx', expected keyword line "
"'parameter_set\n'"
	}
	,
	{
R"file(parameter_set)file"
	,
"Syntax error at line 1, pos 13: 'parameter_set', expected keyword line "
"'parameter_set\n'"
	}
	,
	{
R"file(parameter_set
)file"
	,
"Syntax error at line 2, pos 0: '', expected at least one parameter set "
"line '\tname\n'"
	},{
R"file(parameter_set
	name1
	name2
)file"
	,
"Syntax error at line 3, pos 0: '\tname2\n', expected at least one "
"parameter line '\t\tname = value\n' with name != 'parameter_set'"
	}
	,
	{
R"file(parameter_set
	name1
		parameter_set = a
)file"
	,
"Syntax error at line 3, pos 2: '\t\tparameter_set = a\n', expected a "
"parameter, but a parameter name ('\t\tname = value\n') must not be "
"'parameter_set'"
	}
	,
	{
R"file(parameter_set
	name1
		test=a
	name2
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one "
"parameter line '\t\tname = value\n' with name != 'parameter_set'"
	}
	,
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
)file"
	,
"Syntax error at line 6, pos 0: '', expected keyword line 'module\n'"
	}
	,
	{
R"file(module)file"
	,
"Syntax error at line 1, pos 0: 'module', expected keyword line "
"'parameter_set\n' or keyword line 'module\n'"
	}
	,
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
module)file"
	,
"Syntax error at line 6, pos 6: 'module', expected keyword line 'module\n'"
	}
	,
	{
R"file(parameter_set
	name1
		test=a
	name2
		test=b
module
)file"
	,
"Syntax error at line 7, pos 0: '', expected at least one module line "
"'\tname = module\n'"
	}
	,
	{
R"file(module
)file"
	,
"Syntax error at line 2, pos 0: '', expected at least one module line "
"'\tname = module\n'"
	}
	,
	{
R"file(module
	name3)file"
	,
"Syntax error at line 2, pos 6: '\tname3', expected a module line "
"'\tname = module\n'"
	}
	,
	{
R"file(module
	name3 = module1)file"
	,
"Syntax error at line 2, pos 16: '\tname3 = module1', expected a module line "
"'\tname = module\n'"
	}
	,
	{
R"file(module
	name3 = module1
)file"
	,
"Syntax error at line 3, pos 0: '', expected a parameter_set "
"reference '\t\tparameter_set = name\n', where 'parameter_set' is a keyword "
"and 'name' the name of the referenced parameter set or a parameter "
"'\t\tname = value\n' with name != 'parameter_set' or a module line "
"'\tname = module\n' or keyword line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set =)file"
	,
"Syntax error at line 3, pos 17: '\t\tparameter_set =', expected a "
"parameter_set reference '\t\tparameter_set = name\n', where 'parameter_set' "
"is a keyword and 'name' the name of the referenced parameter set"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set = name1
)file"
	,
"Syntax error at line 4, pos 0: '', expected "
"a parameter_set reference '\t\tparameter_set = name\n', where "
"'parameter_set' is a keyword and 'name' the name of the referenced parameter "
"set or a parameter '\t\tname = value\n' with name != 'parameter_set' or a "
"module line '\tname = module\n' or keyword line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set = name1
			test
)file"
	,
"Syntax error at line 4, pos 0: '\t\t\ttest\n', expected "
"a parameter_set reference '\t\tparameter_set = name\n', where "
"'parameter_set' is a keyword and 'name' the name of the referenced parameter "
"set or a parameter '\t\tname = value\n' with name != 'parameter_set' or a "
"module line '\tname = module\n' or keyword line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
			parameter_set = name1
)file"
	,
"Syntax error at line 3, pos 0: '\t\t\tparameter_set = name1\n', expected a "
"parameter_set reference '\t\tparameter_set = name\n', where 'parameter_set' "
"is a keyword and 'name' the name of the referenced parameter set or a "
"parameter '\t\tname = value\n' with name != 'parameter_set' or a module line "
"'\tname = module\n' or keyword line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set = name1
		test = 5)file"
	,
"Syntax error at line 4, pos 10: '\t\ttest = 5', expected "
"a parameter '\t\tname = value\n' with name != 'parameter_set'"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set = name1
		test1 = 5
		test2 = 2)file"
	,
"Syntax error at line 5, pos 11: '\t\ttest2 = 2', expected "
"a parameter '\t\tname = value\n' with name != 'parameter_set'"
	}
	,
	{
R"file(module
	name3 = module1
		parameter_set = name1
		test1 = 5
		parameter_set = name2
)file"
	,
"Syntax error at line 5, pos 2: '\t\tparameter_set = name2\n', expected "
"a parameter, but a parameter name ('\t\tname = value\n') must not be "
"'parameter_set'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
		parameter_set = name2
)file"
	,
"Syntax error at line 4, pos 2: '\t\tparameter_set = name2\n', expected "
"a parameter, but a parameter name ('\t\tname = value\n') must not be "
"'parameter_set'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
	name4 = module2
)file"
	,
"Syntax error at line 5, pos 0: '', expected a parameter_set reference "
"'\t\tparameter_set = name\n', where 'parameter_set' is a keyword and 'name' "
"the name of the referenced parameter set or a parameter '\t\tname = value\n' "
"with name != 'parameter_set' or a module line '\tname = module\n' or keyword "
"line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain)file"
	,
"Syntax error at line 4, pos 5: 'chain', expected keyword line 'chain\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
)file"
	,
"Syntax error at line 5, pos 0: '', expected at least one chain line "
"'\tname [= group]\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5 =
)file"
	,
"Syntax error at line 5, pos 8: '\tname5 =\n', expected a chain line with "
"group '\tname = group\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		id_generator
)file"
	,
"Syntax error at line 6, pos 14: '\t\tid_generator\n', expected a "
"id_generator line '\t\tid_generator = name\n', where id_generator is a "
"keyword"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		id_generator = x1
		id_generator = x2
)file"
	,
"Syntax error at line 7, pos 14: '\t\tid_generator = x2\n', expected a module "
"'\t\tmodule\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			->
)file"
	,
"Syntax error at line 8, pos 0: '', expected at least one output map "
"'\t\t\t\tparameter = variable'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			->
				name
)file"
	,
"Syntax error at line 8, pos 8: '\t\t\t\tname\n', expected output map "
"'\t\t\t\tparameter = variable'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			<-
)file"
	,
"Syntax error at line 8, pos 0: '', expected at least one input map "
"'\t\t\t\tparameter = variable'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			<-
				name =
)file"
	,
"Syntax error at line 8, pos 10: '\t\t\t\tname =\n', expected input map "
"'\t\t\t\tparameter = variable'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			->)file"
	,
"Syntax error at line 7, pos 5: '\t\t\t->', expected keyword line '\t\t\t<-\n'"
	}
	,
	{
R"file(module
	name3 = module1
		test1 = 5
chain
	name5
		load
			<-)file"
	,
"Syntax error at line 7, pos 5: '\t\t\t<-', expected keyword line '\t\t\t->\n'"
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
