#include <iostream>
#include "ArgParser.h"
#include "Compiler.h"

int main(int argc, char** args)
{
	auto parseResults = ArgParser::parse(argc, args);
	BonsaiCompiler compiler(parseResults.inputFile, parseResults.outputFile, parseResults.hidelinenumbers);
	std::cout << compiler.compile() << std::endl;

	return 0;
}