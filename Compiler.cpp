#include "Compiler.h"
#include <fstream>
#include <iostream>
#include "rang.h"

using std::cout;
using std::endl;

regex easyBonsaiRegex(R"r(^(mov (\d+),\ *(\d+)\ *#\ *(\d+))|(mov (\d+)\ *,\ *NULL)|(goto (.+))|((.+):\ .*)|(jmp ([+-]\d+))|(and (\d+)\ *,\ *(\d+))|((\d+) NAND (\d+))|(or (\d+)\ *,\ *(\d+))|((.*):)$)r");
regex bonsaiValidationRegex(R"r(^(tst \d+)|(jmp \d+)|(inc \d+)|(dec \d+)|(hlt)$)r");

#ifndef BONSAI_WEB
BonsaiCompiler::BonsaiCompiler(string input, string output)
{
	this->m_OutPutFile = output;

	std::ifstream fInput(input);
	string currentLine;
	while (std::getline(fInput, currentLine))
	{
		m_CurrentCode.push_back(currentLine);
	}
}
#endif

#ifdef BONSAI_WEB
BonsaiCompiler::BonsaiCompiler(string& inputLiteral)
{
	this->m_OutPutFile = "";

	static auto split = [](const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> strings;

		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = str.find(delimiter, prev)) != std::string::npos)
		{
			strings.push_back(str.substr(prev, pos - prev));
			prev = pos + 1;
		}

		// To get the last substring (or only, if delimiter is not found)
		strings.push_back(str.substr(prev));

		return strings;
	};
	auto splitted = split(inputLiteral, "\n");
	for (auto line : splitted)
	{
		m_CurrentCode.push_back(line);
	}
}
#endif

bool BonsaiCompiler::validateCode()
{
	for (int index = 0; m_CurrentCode.size() > index; index++)
	{
		auto line = m_CurrentCode[index];

		bool valid = false;
		if (std::regex_match(line, easyBonsaiRegex) || std::regex_match(line, bonsaiValidationRegex))
			valid = true;
		if (!valid)
			m_ErrorStack.push_back("Unknown instruction \"" + line + "\" in line " + std::to_string(index));
	}
	return m_ErrorStack.size() <= 0;
}

bool BonsaiCompiler::compile()
{
	if (!validateCode())
	{
#ifndef BONSAI_WEB
		cout << rang::fg::red << "[ERROR] " << rang::fg::reset << "Invalid Code found!" << endl;
		for (auto error : m_ErrorStack)
		{
			cout << rang::fg::red << "[ERROR] " << rang::fg::reset << error << endl;
}
#endif
		for (auto error : m_ErrorStack)
		{
			printf(error.c_str());
		}
		return false;
	}

	c_Or();
	c_And();
	c_Funcs();
	c_Allocs();
	c_RelativeJmps();

	return m_ErrorStack.size() <= 0;
}
void BonsaiCompiler::c_Or()
{
	static auto isOr = [](string& line, string& a, string& b)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[19].matched)
			{
				a = res[20];
				b = res[21];
				return true;
			}
		}
		return false;
	};

	string a, b;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isOr(line, a, b))
		{
			auto failure = std::to_string(i + 2);
			auto success = std::to_string(i + 1);
			auto allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + a,
				"jmp " + success,
				"goto anotherTest",
				"anotherTest: tst " + b,
				"jmp " + success,
				"jmp " + failure
			};
			m_CurrentCode += func;
			c_Funcs();
		}
	}
}
void BonsaiCompiler::c_And()
{
	static auto isAnd = [](string& line, string& a, string& b)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[13].matched)
			{
				a = res[14];
				b = res[15];
				return true;
			}
		}
		return false;
	};

	string a, b;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isAnd(line, a, b))
		{
			auto failure = std::to_string(i + 2);
			auto success = std::to_string(i + 1);
			auto allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + a,
				"goto aNotNull",
				"jmp " + failure,
				"aNotNull: tst " + b,
				"jmp " + success,
				"jmp " + failure
			};
			m_CurrentCode += func;
			c_Funcs();
		}
	}
}
void BonsaiCompiler::c_Allocs()
{
	static auto isAlloc = [](string& line, string& a, string& b, string& h)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[1].matched)
			{
				a = res[2];
				b = res[3];
				h = res[4];
				return true;
			}
		}
		return false;
	};
	static auto isZeroAlloc = [](string& line, string& a)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[5].matched)
			{
				a = res[6];
				return true;
			}
		}
		return false;
	};

	string a, b, h;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isAlloc(line, a, b, h))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"start: tst " + b,
				"goto bNotNull",
				"goto bNull",
				"bNotNull: inc " + h,
				"inc " + a,
				"dec " + b,
				"goto start",
				"bNull: tst " + h,
				"goto restore",
				"goto resetHelper",
				"restore: dec " + h,
				"inc " + b,
				"goto bNull",
				"resetHelper: mov " + h + ", NULL",
				"jmp " + std::to_string(i + 1)
			};
			m_CurrentCode += func;
			c_Funcs();
		}
	}
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isZeroAlloc(line, a))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + a,
				"goto notNull",
				"jmp " + std::to_string(i + 1),
				"notNull: dec " + a,
				"jmp " + std::to_string(allocAddy)
			};
			m_CurrentCode += func;
			c_Funcs();
		}
	}
}
void BonsaiCompiler::c_Funcs()
{
	m_FuncIndex.clear();

	static auto isFunc = [](string& line, string& name)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[9].matched)
			{
				name = res[10];
				return true;
			}
		}
		return false;
	};
	static auto isFuncT2 = [](string& line, string& name)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[22].matched)
			{
				name = res[23];
				return true;
			}
		}
		return false;
	};
	static auto isFuncJmp = [](string& line, string& name)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[7].matched)
			{
				name = res[8];
				return true;
			}
		}
		return false;
	};

	string funcName;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isFunc(line, funcName) && m_FuncIndex.find(funcName) == m_FuncIndex.end())
		{
			m_CurrentCode[i] = line.substr(funcName.length() + 2);
			m_FuncIndex.insert({ funcName, i });
		}
		if (isFuncT2(line, funcName) && m_FuncIndex.find(funcName) == m_FuncIndex.end())
		{
			m_CurrentCode[i] = "jmp +1";
			m_FuncIndex.insert({ funcName, i+1 });
		}
	}
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isFuncJmp(line, funcName) && m_FuncIndex.find(funcName) != m_FuncIndex.end())
		{
			m_CurrentCode[i] = "jmp " + std::to_string(m_FuncIndex.at(funcName));
		}
	}
}
void BonsaiCompiler::c_RelativeJmps()
{
	static auto isEasyJmp = [](string& line, string& to)
	{
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[11].matched)
			{
				to = res[12];
				return true;
			}
		}
		return false;
	};

	string to;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isEasyJmp(line, to))
		{
			auto direction = to[0];
			auto amount = std::atoi(to.substr(1).c_str());
			if (direction == '+')
			{
				m_CurrentCode[i] = "jmp " + std::to_string(i + amount);
			}
			else
			{
				m_CurrentCode[i] = "jmp " + std::to_string(i - amount);
			}
		}
	}
}

#ifndef BONSAI_WEB
void BonsaiCompiler::writeResults()
{
	if (m_ErrorStack.size() > 0) return;
	cout << rang::fg::green << "[SUCCESS] " << rang::fg::reset << "Code compiled successfuly!" << endl;
	if (m_OutPutFile.size() > 0)
	{
		std::ofstream out(m_OutPutFile);
		for (auto line : m_CurrentCode)
		{
			out << line << endl;
		}
		out.close();
	}
	else
	{
		for (auto line : m_CurrentCode)
		{
			cout << rang::fg::gray << line << rang::fg::reset << endl;
		}
			}
		}
#endif

vector<string>& BonsaiCompiler::getCodeStack()
{
	return this->m_CurrentCode;
}
vector<string>& BonsaiCompiler::getErrorStack()
{
	return this->m_ErrorStack;
}