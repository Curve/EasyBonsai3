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
	string outputFile;
	bool autoHelper = false;
	//bool hidelinenumbers = false;
};

namespace ArgParser
{
	ParseResult parse(int argc, char** args);
}