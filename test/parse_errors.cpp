#include <disposer/parse.hpp>

#include <iostream>
#include <sstream>


int success(){
	std::cout << "success\n";
	return 0;
}

int fail(){
	std::cout << "fail\n";
	return 1;
}

std::istringstream file01(
R"file(xparameter_set)file");
std::string const file01message(
	"Syntax error at line 1, pos 0: 'xparameter_set', expected keyword "
	"'parameter_set' or keyword 'module'");

std::istringstream file02(
R"file(parameter_setx)file");
std::string const file02message(
	"Syntax error at line 1, pos 13: 'parameter_setx', expected newline");

std::istringstream file03(
R"file(parameter_set)file");
std::string const file03message(
	"Syntax error at line 1, pos 13: 'parameter_set', expected newline");

std::istringstream file04(
R"file(parameter_set
)file");
std::string const file04message(
	"Syntax error at line 1, pos 13: 'parameter_set', expected ");


int parse(std::istringstream& file, std::string const& message){
	try{
		disposer::parse(file);
		std::cout << "No exception\n";
		return fail();
	}catch(std::exception const& e){
		if(e.what() == message){
			return success();
		}else{
			std::cout << e.what() << '\n';
			return fail();
		}
	}catch(...){
		std::cout << "Unknown exception\n";
		return fail();
	}
}

int main(){
	return
		parse(file01, file01message) +
		parse(file02, file02message) +
		parse(file03, file03message) +
		parse(file04, file04message);
}
