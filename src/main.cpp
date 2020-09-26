#include <map>
#include <chrono>
#include <fstream>
#include <iostream>
#include "Console.h"
#include "Compiler.h"
#include "Executor.h"
#include <filesystem>
#include "belegpp/belegpp.h"

using namespace beleg::extensions::strings;
using namespace beleg::extensions::containers;

#ifdef BONSAI_WEB
#include <emscripten/bind.h>
auto compile(std::vector<std::string> _code, std::vector<std::uint32_t> usedRegisters = {})
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
inline std::vector<std::string> readFileToVector(const std::string& filename)
{
	std::vector<std::string> result;
	std::ifstream inputFile(filename);
	std::string currentLine;
	while (std::getline(inputFile, currentLine))
	{
		if (currentLine.length() >= 1 && currentLine.substr(0, 1) == ";") continue;
		result.push_back(currentLine);
	}
	return result;
}

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

	if (args | containsKey("run"))
	{
		EasyBonsai::Executor executor;
		if (!std::filesystem::exists(args["run"]))
		{
			Console::error << "Sepcified file not found" << Console::endl;;
			return 1;
		}

		Console::info << "Running " << args["run"] << Console::endl;

		auto input = readFileToVector(args["run"]);
		if (!executor.load(input))
		{
			for (auto err : executor.getErrorstack())
			{
				Console::error << err << Console::endl;
			}
			return 1;
		}

		if (args | containsKey("setVars"))
		{
			auto setVars = args["setVars"];
			if (!(setVars | contains(":")))
			{
				Console::error << "Invalid setVars format" << Console::endl;
				return 1;
			}
			if (setVars | contains(","))
			{
				for (auto var : (setVars | split(",")))
				{
					if (var | contains(":"))
					{
						auto splitted = var | split(":");
						if (std::regex_match(splitted[0], std::regex(R"r(-?[0-9]+)r")) && std::regex_match(splitted[1], std::regex(R"r(-?[0-9]+)r")))
						{
							Console::debug << "Setting Register $" << splitted[0] << " to " << splitted[1] << Console::endl;
							executor.setRegister(std::stoi(splitted[0]), std::stoi(splitted[1]));
						}
					}
				}
			}
			else
			{
				auto splitted = setVars | split(":");
				if (std::regex_match(splitted[0], std::regex(R"r(-?[0-9]+)r")) && std::regex_match(splitted[1], std::regex(R"r(-?[0-9]+)r")))
				{
					Console::debug << "Setting Register $" << splitted[0] << " to " << splitted[1] << Console::endl;
					executor.setRegister(std::stoi(splitted[0]), std::stoi(splitted[1]));
				}
			}
		}

		auto start_time = std::chrono::high_resolution_clock::now();
		auto executionSuccess = executor.run();
		auto end_time = std::chrono::high_resolution_clock::now();

		if (!executionSuccess)
		{
			for (auto err : executor.getErrorstack())
			{
				Console::error << err << Console::endl;
			}
			return 1;
		}

		Console::info << "Execution finished in " << (end_time - start_time) / std::chrono::milliseconds(1) << "ms!" << Console::endl;

		Console::info << "Registers after execution: " << Console::endl;
		for (auto reg : executor.getRegisters())
		{
			Console::info << "[$" << reg.first << "]: " << reg.second << Console::endl;
		}

		return 0;
	}
	else if (!(args | containsKey("input")))
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
	auto input = readFileToVector(args["input"]);

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
	Console::info << "Additional defined registers: " << additionalRegisters[0] << ", " << additionalRegisters[1] << ", " << additionalRegisters[2] << ", " << additionalRegisters[3] << Console::endl;
	Console::info << "Compilation finished in " << (end_time - start_time) / std::chrono::milliseconds(1) << "ms!" << Console::endl;

	std::ofstream output(args["output"]);
	output << (result.second | join("\n"));
	output.close();

	return 0;
}
#endif