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
"Syntax error at line 7, pos 1: '', expected "
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
	for(auto const& v: tests){
		parse(i++, v.first, v.second);
	}
}
