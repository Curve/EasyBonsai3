#pragma once
#include <map>
#include <regex>
#include <array>
#include <string>
#include <vector>
#include "belegpp/belegpp.h"
using namespace beleg::helpers::print;
using namespace beleg::extensions::strings;
using namespace beleg::extensions::containers;

namespace EasyBonsai
{
	namespace internal
	{
		class CommandCollection
		{
			struct Command
			{
				std::regex regex;
				std::uint32_t argCount;
				bool isUseableAddy = true;
			};
		private:
			std::vector<Command> commands;
		public:
			CommandCollection(std::vector<Command> commands) : commands(commands)
			{}
			bool matchesAny(const std::string& what)
			{
				for (auto command : commands)
				{
					if (std::regex_match(what, command.regex))
					{
						return true;
					}
				}
				return false;
			}
			std::optional<Command> getMatching(const std::string& what)
			{
				for (auto command : commands)
				{
					if (std::regex_match(what, command.regex))
					{
						return command;
					}
				}
				return std::nullopt;
			}
			template <std::size_t instruction>
			bool matches(const std::string& what)
			{
				if (std::regex_match(what, commands.at(instruction).regex))
				{
					return true;
				}
				return false;
			}
			template <typename ReturnType = std::string, std::enable_if_t<std::is_arithmetic<ReturnType>::value || std::is_same<std::string, ReturnType>::value>* = nullptr>
			std::vector<ReturnType> getUsedAddys(const std::string& what)
			{
				std::smatch res;
				std::vector<ReturnType> rtn;

				for (auto command : commands)
				{
					if (!command.isUseableAddy)
						continue;

					if (std::regex_match(what, res, command.regex))
					{
						for (int i = 1; command.argCount >= i; i++)
						{
							if constexpr (std::is_arithmetic<ReturnType>::value)
							{
								if (std::regex_match(res[i].str(), std::regex(R"r(-?[0-9]+)r")))
									rtn.push_back(std::stoi(res[i]));
							}
							else
							{
								rtn.push_back(res[i]);
							}
						}
					}
				}

				return rtn;
			}
			template <std::size_t instruction, std::size_t count, typename ReturnType = std::string, std::enable_if_t<std::is_arithmetic<ReturnType>::value || std::is_same<std::string, ReturnType>::value>* = nullptr>
			std::array<ReturnType, count> getArguments(const std::string& what)
			{
				std::smatch res;
				std::array<ReturnType, count> rtn;

				if (std::regex_match(what, res, commands.at(instruction).regex))
				{
					for (int i = 1; count >= i; i++)
					{
						if constexpr (std::is_arithmetic<ReturnType>::value)
						{
							rtn[i - 1] = std::stoi(res[i]);
						}
						else
						{
							rtn[i - 1] = res[i];
						}
					}
				}

				return rtn;
			}
			template <std::size_t count, typename ReturnType = std::string, std::enable_if_t<std::is_arithmetic<ReturnType>::value || std::is_same<std::string, ReturnType>::value>* = nullptr>
			std::array<ReturnType, count> getArguments(const std::string& what)
			{
				std::smatch res;
				std::array<ReturnType, count> rtn;

				for (auto command : commands)
				{
					if (command.argCount != count)
						continue;

					if (std::regex_match(what, res, command.regex))
					{
						for (int i = 1; count >= i; i++)
						{
							if constexpr (std::is_arithmetic<ReturnType>::value)
							{
								rtn[i - 1] = std::stoi(res[i]);
							}
							else
							{
								rtn[i - 1] = res[i];
							}
						}
						break;
					}
				}

				return rtn;
			}
		};
	}
	inline internal::CommandCollection bonsaiRegex({
			{ std::regex(R"r(^tst (\d+)$)r"), 1},
			{ std::regex(R"r(^jmp (\d+)$)r"), 1, false},
			{ std::regex(R"r(^inc (\d+)$)r"), 1},
			{ std::regex(R"r(^dec (\d+)$)r"), 1},
			{ std::regex(R"r(^hlt$)r"), 0}
		});
	inline internal::CommandCollection easyBonsaiRegex(
		{
			{ std::regex(R"r(^(.*):$)r"), 1, false },
			{ std::regex(R"r(^jg (.+)$)r"), 1, false },
			{ std::regex(R"r(^goto (.+)$)r"), 1, false },
			{ std::regex(R"r(^(.+):\ .*$)r"), 1, false },
			{ std::regex(R"r(^jmp \.(.+)$)r"), 1, false },
			{ std::regex(R"r(^jmp ([+-]\d+)$)r"), 1, false},
			{ std::regex(R"r(^je (.+)$)r"), 1, false },
			{ std::regex(R"r(^jl (.+)$)r"), 1, false },
			{ std::regex(R"r(^mov (\d+)\ *,\ *NULL$)r"), 1 },
			{ std::regex(R"r(^reg (.+)\ *,\ *(\d+)$)r"), 2 },
			{ std::regex(R"r(^or (\d+)\ *,\ *(\d+)$)r"), 2 },
			{ std::regex(R"r(^cmp (\d+)\ *,\ *(\d+)$)r"), 2 },
			{ std::regex(R"r(^and (\d+)\ *,\ *(\d+)$)r"), 2 },
			{ std::regex(R"r(^mov (\d+),\ *(\d+)\ *$)r"), 2 }
		});

	enum Instruction { LABELN, JG, GOTO, LABEL, JMPTO, JMPR, JE, JL, MOVN, REG, OR, CMP, AND, MOV, TST = 0, JMP, INC, DEC, HLT };

	class Compiler
	{
	private:
		std::vector<std::string> code;

		std::vector<std::string> errorStack;

		std::string helpRegister;
		std::string cmpRegisters[2];
		std::vector<std::uint32_t> knownAddresses;
		std::map<std::string, std::uint32_t> definedLabels;
		std::map<std::string, std::uint32_t> customAdresses;
	private:
		template <Instruction instruction, std::size_t count>
		static std::pair<bool, std::array<std::string, count>> getInstruction(const std::string& line)
		{
			if (easyBonsaiRegex.matches<instruction>(line))
			{
				return { true, easyBonsaiRegex.getArguments<instruction, count>(line) };
			}

			return { false, {} };
		}
		bool isCodeValid()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];

				if (!(easyBonsaiRegex.matchesAny(line) || bonsaiRegex.matchesAny(line)))
				{
					errorStack.push_back(printfs("Unkown instruction \"%s\" in line %u", line.c_str(), i));
				}
			}
			return errorStack.size() == 0;
		}
		void detectUsedAddresses()
		{
			for (auto& line : code)
			{
				auto args = bonsaiRegex.getUsedAddys<std::uint32_t>(line);
				auto args2 = easyBonsaiRegex.getUsedAddys<std::uint32_t>(line);
				args.insert(args.end(), args2.begin(), args2.end());

				if (args.size() > 0)
				{
					for (auto addy : args)
					{
						if (!(knownAddresses | contains(addy)))
						{
							knownAddresses.push_back(addy);
						}
					}
				}
			}
#ifndef BONSAI_WEB
			Console::debug << "Detected Used-Addresses: { " << (knownAddresses | join(", ")) << " }" << Console::endl;
#endif
			auto maxRegister = *std::max_element(knownAddresses.begin(), knownAddresses.end());

			helpRegister = std::to_string(maxRegister + 1);
			cmpRegisters[0] = std::to_string(maxRegister + 2);
			cmpRegisters[1] = std::to_string(maxRegister + 3);

#ifndef BONSAI_WEB
			Console::debug << "Setting Help-Register to [$0: " << helpRegister << "]" << Console::endl;
			Console::debug << "Setting Compare-Registers to [$1: " << cmpRegisters[0] << "] and [$2: " << cmpRegisters[1] << "]" << Console::endl;
#endif
		}
		void detectMacrosAndLabels()
		{
			std::vector<std::size_t> toDelete;

			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<REG>(line))
				{
					auto args = easyBonsaiRegex.getArguments<REG, 2>(line);
					if (!(customAdresses | containsKey(args[0])))
					{
						customAdresses.insert({ args[0], std::stoi(args[1]) });
					}
					toDelete.push_back(i);
				}
				else if (easyBonsaiRegex.matches<LABELN>(line))
				{
					auto args = easyBonsaiRegex.getArguments<LABELN, 1>(line);
					if (!(definedLabels | containsKey(args[0])))
					{
						definedLabels.insert({ args[0], i + 1 });
					}
					toDelete.push_back(i);
				}
				else if (easyBonsaiRegex.matches<LABEL>(line))
				{
					auto args = easyBonsaiRegex.getArguments<LABEL, 1>(line);
					if (!(definedLabels | containsKey(args[0])))
					{
						line = line.substr(args[0].size() + 2);
						definedLabels.insert({ args[0], i });
					}
				}
			}

			for (int it = 0; toDelete.size() > it; it++)
			{
				auto index = toDelete[it] - it;

				for (const auto& label : definedLabels)
				{
					if (label.second >= index)
					{
						definedLabels.at(label.first) = label.second - 1;
					}
				}
				for (int i = 0; index > i; i++)
				{
					auto& line = code[i];

					if (easyBonsaiRegex.matches<JMPR>(line))
					{
						auto args = easyBonsaiRegex.getArguments<JMPR, 1, std::int32_t>(line);
						if (args[0] >= 0 && (i + args[0]) > index)
							line = "jmp +" + std::to_string(args[0] - 1);
					}
				}
				for (int i = index; code.size() > i; i++)
				{
					auto& line = code[i];

					if (bonsaiRegex.matches<JMP>(line) && std::regex_match(line, std::regex(R"r(-?[0-9]+)r")))
					{
						auto args = bonsaiRegex.getArguments<JMP, 1, std::uint32_t>(line);
							line = "jmp " + std::to_string(args[0] - 1);
					}
					else if (easyBonsaiRegex.matches<JE>(line) && std::regex_match(line, std::regex(R"r(-?[0-9]+)r")))
					{
						auto args = easyBonsaiRegex.getArguments<JE, 1, std::uint32_t>(line);
						line = "je " + std::to_string(args[0] - 1);
					}
					else if (easyBonsaiRegex.matches<JL>(line) && std::regex_match(line, std::regex(R"r(-?[0-9]+)r")))
					{
						auto args = easyBonsaiRegex.getArguments<JL, 1, std::uint32_t>(line);
						line = "jl " + std::to_string(args[0] - 1);
					}
					else if (easyBonsaiRegex.matches<JE>(line) && std::regex_match(line, std::regex(R"r(-?[0-9]+)r")))
					{
						auto args = easyBonsaiRegex.getArguments<JG, 1, std::uint32_t>(line);
						line = "jg " + std::to_string(args[0] - 1);
					}
					else if (easyBonsaiRegex.matches<JMPR>(line))
					{
						auto args = easyBonsaiRegex.getArguments<JMPR, 1, std::int32_t>(line);
						if (args[0] < 0 && (i - std::abs(args[0])) < index)
							line = "jmp " + std::to_string(args[0] + 1);
					}
				}

				code | removeAt(index);
			}

#ifndef BONSAI_WEB
			Console::debug << "Detected Labels: { " << (definedLabels | join(", ")) << " }" << Console::endl;
			Console::debug << "Detected Address-Macros: { " << (customAdresses | join(", ")) << " }" << Console::endl;
#endif

			for (auto& cAddy : customAdresses)
			{
				for (auto& line : code)
				{
					if (line | contains(cAddy.first))
					{
						auto replaced = (line | replace(cAddy.first, std::to_string(cAddy.second)));
						auto easyBonsaiMatch = easyBonsaiRegex.getMatching(replaced);
						auto bonsaiMatch = bonsaiRegex.getMatching(replaced);

						if ((easyBonsaiMatch.has_value() && easyBonsaiMatch.value().isUseableAddy) || (bonsaiMatch.has_value() && bonsaiMatch.value().isUseableAddy))
						{
							line = replaced;
						}
					}
				}
			}
		}
		void handleCmpInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<CMP>(line))
				{
					auto args = easyBonsaiRegex.getArguments<CMP, 2>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);

					std::vector<std::string> cmpFunc =
					{
						/*00*/"mov " + cmpRegisters[0] + "," + args[0],
						/*01*/"mov " + cmpRegisters[1] + "," + args[1],
						/*02*/"tst " + args[0], /*start*/
						/*03*/"jmp " + std::to_string(functionStart + 5),
						/*04*/"jmp " + std::to_string(functionStart + 8),
						/*05*/"tst " + args[1], /*xNotNull*/
						/*06*/"jmp " + std::to_string(functionStart + 11),
						/*07*/"jmp " + std::to_string(functionStart + 19),
						/*08*/"tst " + args[1], /*xNull*/
						/*09*/"jmp " + std::to_string(functionStart + 14),
						/*10*/"jmp " + std::to_string(functionStart + 25),
						/*11*/"dec " + args[0], /*yNotNull*/
						/*12*/"dec " + args[1],
						/*13*/"jmp " + std::to_string(functionStart + 2),
						/*14*/"mov " + args[0] + "," + cmpRegisters[0], /*less*/
						/*15*/"mov " + args[1] + "," + cmpRegisters[1],
						/*16*/"mov " + cmpRegisters[0] + ", NULL",
						/*17*/"mov " + cmpRegisters[1] + ", NULL",
						/*18*/"jmp " + continueExec,
						/*19*/"mov " + args[0] + "," + cmpRegisters[0], /*greater*/
						/*20*/"mov " + args[1] + "," + cmpRegisters[1],
						/*21*/"mov " + cmpRegisters[0] + ", NULL",
						/*22*/"mov " + cmpRegisters[1] + ", NULL",
						/*23*/"inc " + cmpRegisters[0],
						/*24*/"jmp " + continueExec,
						/*25*/"mov " + args[0] + "," + cmpRegisters[0], /*equal*/
						/*26*/"mov " + args[1] + "," + cmpRegisters[1],
						/*27*/"mov " + cmpRegisters[0] + ", NULL",
						/*28*/"mov " + cmpRegisters[1] + ", NULL",
						/*29*/"inc " + cmpRegisters[1],
						/*30*/"jmp " + continueExec
					};
					code.insert(code.end(), cmpFunc.begin(), cmpFunc.end());
				}
				else if (easyBonsaiRegex.matches<JE>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JE, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jeFunc =
					{
						"tst " + cmpRegisters[1],
						"jmp " + args[0],
						"jmp " + continueExec
					};
					code.insert(code.end(), jeFunc.begin(), jeFunc.end());
				}
				else if (easyBonsaiRegex.matches<JL>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JL, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jeFunc =
					{
						"tst " + cmpRegisters[0],
						"jmp " + continueExec,
						"jmp " + args[0]
					};
					code.insert(code.end(), jeFunc.begin(), jeFunc.end());
				}
				else if (easyBonsaiRegex.matches<JG>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JG, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jeFunc =
					{
						"tst " + cmpRegisters[0],
						"jmp " + args[0],
						"jmp " + continueExec
					};
					code.insert(code.end(), jeFunc.begin(), jeFunc.end());
				}
			}
		}
		void handleMovInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<MOV>(line))
				{
					auto args = easyBonsaiRegex.getArguments<MOV, 2>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> movFunc =
					{
						/*00*/"mov " + helpRegister + ", NULL",
						/*01*/"mov " + args[0] + ", NULL",
						/*02*/"tst " + args[1], /*start*/
						/*03*/"jmp " + std::to_string(functionStart + 5),
						/*04*/"jmp " + std::to_string(functionStart + 9),
						/*05*/"inc " + helpRegister, /*bNotNull*/
						/*06*/"inc " + args[0],
						/*07*/"dec " + args[1],
						/*08*/"jmp " + std::to_string(functionStart + 2),
						/*09*/"tst " + helpRegister, /*bNull*/
						/*10*/"jmp " + std::to_string(functionStart + 12),
						/*11*/"jmp " + continueExec,
						/*12*/"dec " + helpRegister, /*restore*/
						/*13*/"inc " + args[1],
						/*14*/"jmp " + std::to_string(functionStart + 9)
					};
					code.insert(code.end(), movFunc.begin(), movFunc.end());
				}
				else if (easyBonsaiRegex.matches<MOVN>(line))
				{
					auto args = easyBonsaiRegex.getArguments<MOVN, 1>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> movZeroFunc =
					{
						/*0*/"tst " + args[0],
						/*1*/"jmp " + std::to_string(functionStart + 3),
						/*2*/"jmp " + continueExec,
						/*3*/"dec " + args[0],
						/*4*/"jmp " + std::to_string(functionStart + 0)
					};
					code.insert(code.end(), movZeroFunc.begin(), movZeroFunc.end());
				}
			}
		}
		void handleOrInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<OR>(line))
				{
					auto args = easyBonsaiRegex.getArguments<OR, 2>(line);

					auto functionStart = code.size();
					auto trueBranch = std::to_string(i + 1);
					auto falseBranch = std::to_string(i + 2);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> orFunc =
					{
						/*0*/"tst " + args[0],
						/*1*/"jmp " + trueBranch,
						/*2*/"jmp " + std::to_string(functionStart + 3),
						/*3*/"tst " + args[1],
						/*4*/"jmp " + trueBranch,
						/*5*/"jmp " + falseBranch
					};
					code.insert(code.end(), orFunc.begin(), orFunc.end());
				}
			}
		}
		void handleAndInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<AND>(line))
				{
					auto args = easyBonsaiRegex.getArguments<AND, 2>(line);

					auto functionStart = code.size();
					auto trueBranch = std::to_string(i + 1);
					auto falseBranch = std::to_string(i + 2);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> orFunc =
					{
						/*0*/"tst " + args[0],
						/*1*/"jmp " + std::to_string(functionStart + 3),
						/*2*/"jmp " + falseBranch,
						/*3*/"tst " + args[1],
						/*4*/"jmp " + trueBranch,
						/*5*/"jmp " + falseBranch
					};
					code.insert(code.end(), orFunc.begin(), orFunc.end());
				}
			}
		}
		void handleGotoInstruction()
		{
			for (auto& line : code)
			{
				if (easyBonsaiRegex.matches<GOTO>(line))
				{
					auto args = easyBonsaiRegex.getArguments<GOTO, 1>(line);
					if (definedLabels | containsKey(args[0]))
					{
						line = "jmp " + std::to_string(definedLabels.at(args[0]));
					}
				}
				else if (easyBonsaiRegex.matches<JMPTO>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JMPTO, 1>(line);
					if (definedLabels | containsKey(args[0]))
					{
						line = "jmp " + std::to_string(definedLabels.at(args[0]));
					}
				}
			}
		}
	public:
		Compiler() {}
		std::pair<bool, std::vector<std::string>> compile(std::vector<std::string> _code, std::vector<std::uint32_t> usedRegisters = {})
		{
			code = _code;
			knownAddresses.insert(knownAddresses.end(), usedRegisters.begin(), usedRegisters.end());

			/*
				Detect-Macros has to be run first, because the validation does not account for macros inside of the instruction.
			*/

			detectMacrosAndLabels();

			if (!isCodeValid())
				return { false, errorStack };

			detectUsedAddresses();

			handleCmpInstruction();
			handleMovInstruction();
			handleAndInstruction();
			handleOrInstruction();

			handleGotoInstruction();

			return { true, code };
		}
		std::array<std::uint32_t, 3> getNeededRegisters()
		{
			return { (std::uint32_t)std::stoi(cmpRegisters[0]), (std::uint32_t)std::stoi(cmpRegisters[1]), (std::uint32_t)std::stoi(helpRegister) };
		}
	};
}