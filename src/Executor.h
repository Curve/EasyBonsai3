#pragma once
#include <map>
#include <string>
#include <vector>
#include <variant>
#include "Console.h"
#include "Compiler.h"
#include "belegpp/belegpp.h"

namespace EasyBonsai
{
	class Executor
	{
	private:
		std::vector<std::string> code;
		std::vector<std::string> errorStack;
		std::map<std::uint32_t, std::uint32_t> registers;
	public:
		Executor() { }
		auto getErrorstack()
		{
			return errorStack;
		}
		auto getRegisters()
		{
			return registers;
		}
		void setRegister(std::uint32_t id, std::uint32_t value)
		{
			registers[id] = value;
		}
		bool load(std::vector<std::string> _code)
		{
			code = _code;
			for (int i = 0; code.size() > i; i++)
			{
				const auto& line = code[i];
				if (!EasyBonsai::bonsaiRegex.matchesAny(line))
				{
					errorStack.push_back(printfs("Unkown instruction \"%s\" in line %u", line.c_str(), i));
				}
				else
				{
					auto args = EasyBonsai::bonsaiRegex.getUsedAddys<std::uint32_t>(line);
					for (auto arg : args)
					{
						registers[arg] = 0;
					}
				}
			}

			return errorStack.size() <= 0;
		}
		bool run()
		{
			using namespace EasyBonsai;

			std::uint32_t ip = 0;
			while (true)
			{
				const auto& line = code[ip++];

#ifndef BONSAI_WEB
				if (bonsaiRegex.matches<INT>(line))
				{
					Console::debug << "Interrupt called! Registers: " << (registers | join(",")) << Console::endl;
					std::cin.get();
				}
#endif
				if (bonsaiRegex.matches<TST>(line))
				{
					auto args = bonsaiRegex.getArguments<TST, 1, std::uint32_t>(line);
					if (registers.at(args[0]) <= 0)
					{
						ip++;
					}
				}
				if (bonsaiRegex.matches<JMP>(line))
				{
					auto args = bonsaiRegex.getArguments<JMP, 1, std::uint32_t>(line);
					auto newIP = args[0];
					if (newIP >= code.size() || newIP < 0)
					{
						errorStack.push_back(printfs("Jmp Destination out of bounds in line %u", ip));
						return false;
					}
					ip = args[0];
				}
				if (bonsaiRegex.matches<INC>(line))
				{
					auto args = bonsaiRegex.getArguments<INC, 1, std::uint32_t>(line);
					registers.at(args[0])++;
				}
				if (bonsaiRegex.matches<DEC>(line))
				{
					auto args = bonsaiRegex.getArguments<DEC, 1, std::uint32_t>(line);
					registers.at(args[0])--;
				}
				if (bonsaiRegex.matches<HLT>(line) || ip >= code.size())
				{
					break;
				}
			}

			return true;
		}
	};
}