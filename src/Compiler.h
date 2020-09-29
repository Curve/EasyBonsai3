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
				std::vector<std::uint32_t> ignoredAddys;
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
							if (command.ignoredAddys | contains(i))
								continue;
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
			{ std::regex(R"r(^hlt$)r"), 0, false},
			{ std::regex(R"r(^int$)r"), 0, false }
		});
	inline internal::CommandCollection easyBonsaiRegex(
		{
			{ std::regex(R"r(^([a-zA-Z0-9-_]+):$)r"), 1, false },
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
			{ std::regex(R"r(^mov (\d+),\ *(\d+)\ *$)r"), 2 },
			{ std::regex(R"r(^add (\d+),\ *(\d+)\ *$)r"), 2 },
			{ std::regex(R"r(^sub (\d+),\ *(\d+)\ *$)r"), 2 },
			{ std::regex(R"r(^inc (\d+),\ *(\d+)\ *$)r"), 2, true, {2} },
			{ std::regex(R"r(^dec (\d+),\ *(\d+)\ *$)r"), 2, true, {2} },
			{ std::regex(R"r(^jne (.+)$)r"), 1, false },
			{ std::regex(R"r(^ret$)r"), 0, false },
			{ std::regex(R"r(^ret (.+)$)r"), 1 },
			{ std::regex(R"r(^\(fun (.+)\(([a-zA-Z0-9, ]*)\):\ *$)r"), 2, false },
			{ std::regex(R"r(^push (.+)$)r"), 2 },
			{ std::regex(R"r(^call (.+)$)r"), 1, false }
		});

	enum Instruction { LABELN, JG, GOTO, LABEL, JMPTO, JMPR, JE, JL, MOVN, REG, OR, CMP, AND, MOV, ADD, SUB, VINC, VDEC, JNE, RET, RETV, FUNCDEF, PUSH, CALL, TST = 0, JMP, INC, DEC, HLT, INT };

	class Compiler
	{
		struct Function
		{
			std::string name;
			std::vector<std::string> code;
			std::vector<std::string> params;
		};
	private:
		std::vector<std::string> code;

		std::vector<std::string> errorStack;

		std::string returnRegister;
		std::string cmpRegisters[2];
		std::string helpRegisters[2];
		std::vector<std::uint32_t> knownAddresses;
		std::map<std::string, std::uint32_t> definedLabels;
		std::map<std::string, Function> functionDefintions;
		std::map<std::string, std::uint32_t> customAdresses;
	private:
		bool usesVariable(std::string& line, const std::string& variableName)
		{
			std::string rtn = "";
			auto splitted = line | split(" ");
			for (int j = 0; splitted.size() > j; j++)
			{
				auto& split = splitted[j];
				auto trimmed = split | trim();
				auto replaced = (trimmed | replace(",", ""));

				if (replaced == variableName)
				{
					return true;
				}
			}
			return false;
		}
		std::string formatAndReplace(std::string& line, const std::string& variableName, const std::string& variableValue)
		{
			std::string rtn = "";
			auto splitted = line | split(" ");
			for (int j = 0; splitted.size() > j; j++)
			{
				auto& split = splitted[j];
				auto trimmed = split | trim();
				auto replaced = (trimmed | replace(",", ""));

				if (replaced == variableName)
				{
					rtn += ((trimmed | startsWith(",")) ? "," : "") + variableValue;
				}
				else
				{
					rtn += replaced;
				}

				if (trimmed | endsWith(","))
					rtn += ",";

				if (j != (splitted.size() - 1))
					rtn += " ";
			}
			return rtn;
		}
		void deleteLines(std::vector<std::size_t> lines)
		{
			for (int it = 0; lines.size() > it; it++)
			{
				auto index = lines[it] - it;

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
				for (int i = 0; code.size() > i; i++)
				{
					auto& line = code[i];

					if (i >= index && easyBonsaiRegex.matches<JMPR>(line))
					{
						auto args = easyBonsaiRegex.getArguments<JMPR, 1, std::int32_t>(line);
						if (args[0] < 0 && (i - std::abs(args[0])) < index)
							line = "jmp " + std::to_string(args[0] + 1);
					}
					else if (std::regex_search(line, std::regex(R"r([0-9]+)r")))
					{
						if (bonsaiRegex.matches<JMP>(line))
						{
							auto args = bonsaiRegex.getArguments<JMP, 1, std::uint32_t>(line);
							if (args[0] >= index)
								line = "jmp " + std::to_string(args[0] - 1);
						}
						else if (easyBonsaiRegex.matches<JE>(line))
						{
							auto args = easyBonsaiRegex.getArguments<JE, 1, std::uint32_t>(line);
							if (args[0] >= index)
								line = "je " + std::to_string(args[0] - 1);
						}
						else if (easyBonsaiRegex.matches<JNE>(line))
						{
							auto args = easyBonsaiRegex.getArguments<JNE, 1, std::uint32_t>(line);
							if (args[0] >= index)
								line = "jne " + std::to_string(args[0] - 1);
						}
						else if (easyBonsaiRegex.matches<JL>(line))
						{
							auto args = easyBonsaiRegex.getArguments<JL, 1, std::uint32_t>(line);
							if (args[0] >= index)
								line = "jl " + std::to_string(args[0] - 1);
						}
						else if (easyBonsaiRegex.matches<JG>(line))
						{
							auto args = easyBonsaiRegex.getArguments<JG, 1, std::uint32_t>(line);
							if (args[0] >= index)
								line = "jg " + std::to_string(args[0] - 1);
						}
					}
				}
				code | removeAt(index);
			}
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
			for (auto line : code)
			{
				if (usesVariable(line, "eax"))
				{
					line = formatAndReplace(line, "eax", "0");
				}

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
			std::uint32_t maxRegister = 0;
			if (knownAddresses.size() > 0)
				maxRegister = *std::max_element(knownAddresses.begin(), knownAddresses.end());

			helpRegisters[0] = std::to_string(maxRegister + 2);
			helpRegisters[1] = std::to_string(maxRegister + 3);
			cmpRegisters[0] = std::to_string(maxRegister + 4);
			cmpRegisters[1] = std::to_string(maxRegister + 5);

			returnRegister = std::to_string(maxRegister + 1);
			customAdresses.insert({ "eax", maxRegister + 1 });

			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (usesVariable(line, "eax"))
				{
					line = formatAndReplace(line, "eax", returnRegister);
				}
			}

#ifndef BONSAI_WEB
			Console::debug << "Setting Eax-Register to [" << returnRegister << "]" << Console::endl;
			Console::debug << "Setting Help-Registers to [" << helpRegisters[0] << ", " << helpRegisters[1] << "]" << Console::endl;
			Console::debug << "Setting Compare-Registers to [" << cmpRegisters[0] << ", " << cmpRegisters[1] << "]" << Console::endl;
#endif
		}
		void handleFunctions()
		{
			std::vector<std::size_t> toDelete;
			bool inFunction = false;
			Function currentFunction;

			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<FUNCDEF>(line))
				{
					if (inFunction)
					{
						errorStack.push_back(printfs("Trying to define function inside of function in line %u", i));
						return;
					}

					auto args = easyBonsaiRegex.getArguments<FUNCDEF, 2>(line);
					currentFunction.name = args[0];

					auto splitted = args[1] | split(",");
					for (auto& split : splitted)
					{
						if (customAdresses | containsKey(split | trim()))
						{
							errorStack.push_back(printfs("Function Parameter \"%s\" defined in line %u conflicts with variable of the same name", split.c_str(), i));
							return;
						}
						currentFunction.params.push_back(split | trim());
					}

					inFunction = true;
					toDelete.push_back(i);
				}
				else if (inFunction)
				{
					if ((line | trim()) == ")")
					{
						inFunction = false;
						toDelete.push_back(i);
						functionDefintions.insert({ currentFunction.name | trim(), currentFunction });
						currentFunction.code.clear();
						currentFunction.params.clear();
					}
					else
					{
						toDelete.push_back(i);
						currentFunction.code.push_back(line);
					}
				}
			}

			deleteLines(toDelete);
			toDelete.clear();

			bool wasPreviousPush = false;
			std::vector<std::string> pushStack;
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];

				if (easyBonsaiRegex.matches<PUSH>(line))
				{
					wasPreviousPush = true;
					auto args = easyBonsaiRegex.getArguments<PUSH, 1>(line);
					pushStack.push_back(args[0]);
					toDelete.push_back(i);
				}
				else if (easyBonsaiRegex.matches<CALL>(line))
				{
					wasPreviousPush = false;
					auto args = easyBonsaiRegex.getArguments<CALL, 1>(line);
					if (!(functionDefintions | containsKey(args[0] | trim())))
					{
						errorStack.push_back(printfs("Tried to call non existant function \"%s\" in line %u", args[0].c_str(), i));
					}
					else
					{
						auto functionStart = code.size();
						auto continueExec = std::to_string(i + 1);
						auto functionInfo = functionDefintions.at(args[0] | trim());

						line = "jmp " + std::to_string(functionStart);

						if (pushStack.size() != functionInfo.params.size())
						{
							errorStack.push_back(printfs("Too few arguments for function \"%s\" provided on function call \"%s\" in line %u", functionInfo.name.c_str(), line, i));
							return;
						}

						std::vector<std::string> newCode = functionInfo.code;
						for (int k = 0; newCode.size() > k; k++)
						{
							auto& newLine = newCode[k];
							for (int j = 0; functionInfo.params.size() > j; j++)
							{
								auto param = functionInfo.params[j];
								auto paramValue = pushStack[j];

								newLine = formatAndReplace(newLine, param, paramValue);
							}

							if (easyBonsaiRegex.matches<RET>(newLine | trim()))
							{
								newLine = "jmp " + continueExec;
							}
							else if (easyBonsaiRegex.matches<RETV>(newLine | trim()))
							{
								auto retArg = easyBonsaiRegex.getArguments<RETV, 1>(newLine);
								std::vector<std::string> returnFunc =
								{
									"mov eax, " + retArg[0],
									"jmp " + continueExec
								};
								newLine = "jmp " + std::to_string(code.size() + newCode.size());
								newCode.insert(newCode.end(), returnFunc.begin(), returnFunc.end());
							}
						}
						code.insert(code.end(), newCode.begin(), newCode.end());
						pushStack.clear();
					}
				}
				else
				{
					if (wasPreviousPush)
					{
						errorStack.push_back(printfs("Expected Call but got \"%s\" in line %u", line.c_str(), i));
						return;
					}
					wasPreviousPush = false;
				}
			}

			deleteLines(toDelete);
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
						customAdresses.insert({ args[0] | trim(), std::stoi(args[1]) });
					}
					else
					{
						errorStack.push_back(printfs("Trying to register variable \"%s\" in line %u but it's already defined", args[0].c_str(), i));
						return;
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
					else
					{
						errorStack.push_back(printfs("Trying to register label \"%s\" in line %u but it's already defined", args[0].c_str(), i));
						return;
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
					else
					{
						errorStack.push_back(printfs("Trying to register label \"%s\" in line %u but it's already defined", args[0].c_str(), i));
						return;
					}
				}
			}
			deleteLines(toDelete);

			handleFunctions();

#ifndef BONSAI_WEB
			Console::debug << "Detected Labels: { " << (definedLabels | join(", ")) << " }" << Console::endl;
			Console::debug << "Detected Address-Macros: { " << (customAdresses | join(", ")) << " }" << Console::endl;
#endif

			std::vector<std::pair<std::uint32_t, std::string>> originals;
			for (auto& cAddy : customAdresses)
			{
				for (int i = 0; code.size() > i; i++)
				{
					auto& line = code[i];
					if (usesVariable(line, cAddy.first))
					{
						originals.push_back({ i, line });
						line = formatAndReplace(line, cAddy.first, std::to_string(cAddy.second));
					}
				}
			}

			for (auto& original : originals)
			{
				auto line = code[original.first];
				if (usesVariable(line, "eax"))
				{
					line = formatAndReplace(line, "eax", "0");
				}

				auto easyBonsaiMatch = easyBonsaiRegex.getMatching(line);
				auto bonsaiMatch = bonsaiRegex.getMatching(line);

				if (!((easyBonsaiMatch.has_value() && easyBonsaiMatch.value().isUseableAddy) || (bonsaiMatch.has_value() && bonsaiMatch.value().isUseableAddy)))
				{
					errorStack.push_back(printfs("Variable is used with incompatible instruction \"%s\" in line %u - Original: %s", original.second.c_str(), original.first, line.c_str()));
					line = original.second;
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
				else if (easyBonsaiRegex.matches<JNE>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JNE, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jneFunc =
					{
						"tst " + cmpRegisters[1],
						"jmp " + continueExec,
						"jmp " + args[0]
					};
					code.insert(code.end(), jneFunc.begin(), jneFunc.end());
				}
				else if (easyBonsaiRegex.matches<JL>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JL, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jlFunc =
					{
						"tst " + cmpRegisters[0],
						"jmp " + continueExec,
						"jmp " + args[0]
					};
					code.insert(code.end(), jlFunc.begin(), jlFunc.end());
				}
				else if (easyBonsaiRegex.matches<JG>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JG, 1>(line);
					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> jgFunc =
					{
						"tst " + cmpRegisters[0],
						"jmp " + args[0],
						"jmp " + continueExec
					};
					code.insert(code.end(), jgFunc.begin(), jgFunc.end());
				}
			}
		}
		void handleMathInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<SUB>(line))
				{
					auto args = easyBonsaiRegex.getArguments<SUB, 2>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> subFunc =
					{
						/*0*/"mov " + helpRegisters[1] + ", " + args[1],
						/*1*/"tst " + helpRegisters[1],
						/*2*/"jmp " + std::to_string(functionStart + 4),
						/*3*/"jmp " + continueExec,
						/*4*/"dec " + args[0],
						/*4*/"dec " + helpRegisters[1],
						/*5*/"jmp " + std::to_string(functionStart + 1)
					};
					code.insert(code.end(), subFunc.begin(), subFunc.end());
				}
				else if (easyBonsaiRegex.matches<ADD>(line))
				{
					auto args = easyBonsaiRegex.getArguments<ADD, 2>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> addFunc =
					{
						/*0*/"mov " + helpRegisters[1] + "," + args[1],
						/*1*/"tst " + helpRegisters[1],
						/*2*/"jmp " + std::to_string(functionStart + 4),
						/*3*/"jmp " + continueExec,
						/*4*/"inc " + args[0],
						/*5*/"dec " + helpRegisters[1],
						/*6*/"jmp " + std::to_string(functionStart + 1)
					};
					code.insert(code.end(), addFunc.begin(), addFunc.end());
				}
				else if (easyBonsaiRegex.matches<VINC>(line))
				{
					auto args = easyBonsaiRegex.getArguments<VINC, 2, std::uint32_t>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> incFunc;
					for (int i = 0; args[1] > i; i++)
					{
						incFunc.push_back("inc " + std::to_string(args[0]));
					}
					incFunc.push_back("jmp " + continueExec);
					code.insert(code.end(), incFunc.begin(), incFunc.end());
				}
				else if (easyBonsaiRegex.matches<VDEC>(line))
				{
					auto args = easyBonsaiRegex.getArguments<VDEC, 2, std::uint32_t>(line);

					auto functionStart = code.size();
					auto continueExec = std::to_string(i + 1);

					line = "jmp " + std::to_string(functionStart);
					std::vector<std::string> decFunc;
					for (int i = 0; args[1] > i; i++)
					{
						decFunc.push_back("dec " + std::to_string(args[0]));
					}
					decFunc.push_back("jmp " + continueExec);
					code.insert(code.end(), decFunc.begin(), decFunc.end());
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
						/*00*/"mov " + helpRegisters[0] + ", NULL",
						/*01*/"mov " + args[0] + ", NULL",
						/*02*/"tst " + args[1], /*start*/
						/*03*/"jmp " + std::to_string(functionStart + 5),
						/*04*/"jmp " + std::to_string(functionStart + 9),
						/*05*/"inc " + helpRegisters[0], /*bNotNull*/
						/*06*/"inc " + args[0],
						/*07*/"dec " + args[1],
						/*08*/"jmp " + std::to_string(functionStart + 2),
						/*09*/"tst " + helpRegisters[0], /*bNull*/
						/*10*/"jmp " + std::to_string(functionStart + 12),
						/*11*/"jmp " + continueExec,
						/*12*/"dec " + helpRegisters[0], /*restore*/
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
		void handleRJMPInstruction()
		{
			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (easyBonsaiRegex.matches<JMPR>(line))
				{
					auto args = easyBonsaiRegex.getArguments<JMPR, 1, std::int32_t>(line);
					line = "jmp " + std::to_string(i + args[0]);
				}
			}
		}
	public:
		Compiler() {}
		std::pair<bool, std::vector<std::string>> compile(std::vector<std::string> _code, std::vector<std::uint32_t> usedRegisters = {})
		{
			code = _code;

			{
				std::vector<std::uint32_t> toDelete;
				for (int i = 0; code.size() > i; i++)
				{
					auto& line = code[i];
					if (!line || (line | startsWith(";")))
					{
						toDelete.push_back(i);
					}
				}
				for (int it = 0; toDelete.size() > it; it++)
				{
					auto index = toDelete[it] - it;
					code | removeAt(index);
				}
			}

			knownAddresses.insert(knownAddresses.end(), usedRegisters.begin(), usedRegisters.end());

			/*
				Detect-Macros has to be run first, because the validation does not account for macros inside of the instruction.
			*/

			detectMacrosAndLabels();
			detectUsedAddresses();

			if (!isCodeValid())
				return { false, errorStack };

			handleMathInstruction();
			handleCmpInstruction();
			handleMovInstruction();
			handleAndInstruction();
			handleOrInstruction();

			handleGotoInstruction();
			handleRJMPInstruction();

			for (int i = 0; code.size() > i; i++)
			{
				auto& line = code[i];
				if (!bonsaiRegex.matchesAny(line))
				{
					errorStack.push_back(printfs("Unexpected instruction \"%s\" in line %u", line.c_str(), i));
				}
			}
			if (errorStack.size() > 0)
				return { false, errorStack };

			return { true, code };
		}
		std::vector<std::uint32_t> getNeededRegisters()
		{
			return { (std::uint32_t)std::stoi(cmpRegisters[0]), (std::uint32_t)std::stoi(cmpRegisters[1]), (std::uint32_t)std::stoi(helpRegisters[0]), (std::uint32_t)std::stoi(helpRegisters[1]), (std::uint32_t)std::stoi(returnRegister) };
		}
	};
}