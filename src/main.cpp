#include <map>
#include <chrono>
#include <fstream>
#include <iostream>
#include "Console.h"
#include "Compiler.h"
#include <filesystem>
#include "belegpp/belegpp.h"

using namespace beleg::extensions::strings;
using namespace beleg::extensions::containers;

#ifdef BONSAI_WEB
#include <emscripten/bind.h>
std::tuple<bool, std::vector<std::string>, std::array<std::uint32_t, 3>> compile(std::vector<std::string> _code, std::vector<std::uint32_t> usedRegisters = {})
{
	auto compiler = EasyBonsai::Compiler();
	auto result = compiler.compile(_code, usedRegisters);
	return std::make_tuple(result.first, result.second, compiler.getNeededRegisters());
}
EMSCRIPTEN_BINDINGS(mygetcode) {
	emscripten::register_vector<std::string>("StringList");
	emscripten::function("compile", &compile);
}
#endif

#ifndef BONSAI_WEB
int main(int argc, char** cargs)
{
	std::map<std::string, std::string> args;

	if (argc <= 1)
	{
		Console::error << "Too few arguments provided" << Console::endl;
		return 1;
	}

	for (int i = 1; argc > i; i++)
	{
		std::string currentArg(cargs[i]);
		if (currentArg | startsWith("--"))
		{
			if (argc > (i + 1) && !(cargs[i + 1] | startsWith("--")))
			{
				args.insert({ currentArg.substr(2), cargs[i + 1] });
			}
			else
			{
				args.insert({ currentArg.substr(2), "" });
			}
		}
	}

	if (!(args | containsKey("input")))
	{
		Console::error << "No input specified" << Console::endl;;
		return 1;
	}
	else if (!std::filesystem::exists(args["input"]))
	{
		Console::error << "No input specified" << Console::endl;;
		return 1;
	}
	if (!(args | containsKey("output")))
	{
		args["output"] = "output.bon";
	}
	else if (!args["output"])
	{
		args["output"] = "output.bon";
	}

	Console::info << "Using input file: " << args["input"] << Console::endl;
	Console::info << "Using output file: " << args["output"] << Console::endl;

	std::vector<std::uint32_t> preDefinedVars = {};
	if (args | containsKey("usedVars"))
	{
		auto vars = args["usedVars"];
		if (vars | contains(","))
		{
			std::vector<std::string> _splitted = (vars | split(","));
			std::vector<std::uint32_t> splitted;
			for (auto& item : _splitted)
			{
				if (std::regex_match(item, std::regex(R"r([0-9]+)r")))
				{
					splitted.push_back(std::stoi(item));
				}
			}
			preDefinedVars.insert(preDefinedVars.end(), splitted.begin(), splitted.end());
		}
		else
		{
			if (std::regex_match(vars, std::regex(R"r([0-9]+)r")))
			{
				preDefinedVars.push_back(std::stoi(vars));
			}
		}
	}

	Console::info << "User defined variables: " << (preDefinedVars | join(", ")) << Console::endl;

	auto compiler = EasyBonsai::Compiler();

	std::vector<std::string> input;
	{
		std::ifstream inputFile(args["input"]);
		std::string currentLine;
		while (std::getline(inputFile, currentLine))
		{
			if (currentLine.length() >= 1 && currentLine.substr(0, 1) == ";") continue;
			input.push_back(currentLine);
		}
	}

	auto start_time = std::chrono::high_resolution_clock::now();
	auto result = compiler.compile(input, preDefinedVars);
	auto end_time = std::chrono::high_resolution_clock::now();

	if (!result.first)
	{
		for (auto err : result.second)
		{
			Console::error << err << Console::endl;
		}
		Console::info << "Compilation failed, terminating" << Console::endl;
		return 1;
	}

	auto additionalRegisters = compiler.getNeededRegisters();
	Console::info << "Additional defined registers: " << additionalRegisters[0] << ", " << additionalRegisters[1] << ", " << additionalRegisters[2] << Console::endl;
	Console::info << "Compilation finished in " << (end_time - start_time) / std::chrono::milliseconds(1) << "ms!" << Console::endl;

	std::ofstream output(args["output"]);
	output << (result.second | join("\n"));
	output.close();

	return 0;
}
#endif