#include <iostream>
#include "ArgParser.h"
#include "Compiler.h"

#ifdef BONSAI_WEB
#include <emscripten/bind.h>
vector<string> getCode(std::string code, bool autoHelper = true) {
	BonsaiCompiler compiler(code);
	compiler.setAutoHelperFlag(autoHelper);

	if (compiler.compile())
	{
		auto codeStack = compiler.getCodeStack();
		return codeStack;
	}
	return compiler.getErrorStack();
}
EMSCRIPTEN_BINDINGS(mygetcode) {
	emscripten::register_vector<std::string>("StringList");
	emscripten::function("getCode", &getCode);
}
#endif

#ifndef BONSAI_WEB
int main(int argc, char** args)
{
	auto parseResults = ArgParser::parse(argc, args);
	BonsaiCompiler compiler(parseResults.inputFile, parseResults.outputFile);
	compiler.setAutoHelperFlag(parseResults.autoHelper);
	compiler.compile();
	compiler.writeResults();

	return 0;
}
#endif