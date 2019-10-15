#include "ArgParser.h"
#include "rang.h"
regex parservalidator(R"r(^(?=.)(((--input|-i) ([a-zA-Z0-9.].*[a-zA-Z0-9.]{1,}))?|((--output|-o) ([a-zA-Z0-9.].*[a-zA-Z0-9.]{1,}))?|(--hidelinenumbers|-hln)?|(--help|-h)?|(-ah|--auto-helper)?)$)r");
#include <iostream>
using std::cout;
using std::endl;

#ifndef BONSAI_WEB
ParseResult ArgParser::parse(int argc, char** args)
{
	if (argc < 2)
	{
		cout << rang::fg::red;
		cout << "[ERROR] ";
		cout << rang::fg::reset;
		cout << "Too few arguments!";
		exit(1);
	}

	ParseResult result;
	for (int i = 1; argc > i; i++)
	{
		smatch regex;
		string arg = args[i];
		if (regex_match(arg, regex, parservalidator))
		{
			//if (regex[8].matched)
			//{
			//	result.hidelinenumbers = true;
			//}
			if (regex[9].matched)
			{
				cout << rang::fg::yellow;
				cout << "Help:" << endl;
				cout << rang::fg::reset;
				cout << "--input (-i) [file]\t\t\tSpecifies the input file [crucial]" << endl;
				cout << "--output (-o) [file]\t\t\tSpecifies the output file, if non is defined output will be print to console" << endl;
				cout << "--auto-helper (-ah)\t\t\tAutomatically determine the address of the helper value" << endl;
				//cout << "--hidelinenumbers (-hln) [file]\t\tHides the line numbers in the console output" << endl;
				exit(0);
			}
			if (regex[10].matched)
			{
				result.autoHelper = true;
			}
		}
	}

	for (int i = 1; argc > i; i++)
	{
		if (i + 1 == argc)
			break;
		smatch regex;
		string arg = string(args[i]) + " " + string(args[i + 1]);
		if (regex_match(arg, regex, parservalidator))
		{
			if (regex[6].matched)
			{
				result.outputFile = regex[7];
			}
			if (regex[3].matched)
			{
				result.inputFile = regex[4];
			}
		}
	}

	if (result.inputFile.length() <= 0)
	{
		cout << rang::fg::red;
		cout << "[ERROR] ";
		cout << rang::fg::reset;
		cout << "No input specified!";
		exit(1);
	}

	return result;
}
#endif