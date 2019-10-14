#pragma once
#include <regex>
#include <string>
using std::pair;
using std::regex;
using std::smatch;
using std::string;
using std::make_pair;
using std::regex_match;

extern regex parservalidator;

struct ParseResult
{
	string inputFile;
	bool hidelinenumbers = false;
	string outputFile = "output.bs";
};

namespace ArgParser
{
	ParseResult parse(int argc, char** args);
}