#include "Compiler.h"
#include <fstream>
#include "rang.h"

using std::cout;
using std::endl;

//TODO: One last check to be certain, that no unknown lines remain!

regex easyBonsaiRegex(R"r(^(\[(\d+)=(\d+)#(\d+)\])|(\[(\d+)=zero\])|(jmp (\w+))|((\w+):\ .*)|(jmp ([+-]\d+))|((\d+) AND (\d+))|((\d+) NAND (\d+))|((\d+) OR (\d+))$)r");
//
//! [0(2)=1(3)#2(4)]		Group 1
//! [0(6)=zero]				Group 5
//! jmp func1(8)			Group 7
//! func1(10): test			Group 9
//! jmp +5(12)				Group 11
//! 10(14) AND 20(15)		Group 12
//! 10(17) NAND 30(18)		Group 16
//! 40(20) OR 40(21)		Group 19
//

regex bonsaiValidationRegex(R"r(^(tst \d+)|(jmp \d+)|(inc \d+)|(dec \d+)|(hlt)$)r");
//
//! tst 1
//! jmp 3
//! jmp 6
//! dec 1
//! inc 0
//! jmp 0
//! hlt
//

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
BonsaiCompiler::BonsaiCompiler(string input, string output, bool hln)
{
	this->m_OutPutFile = output;
	this->m_HideLineNumbers = hln;

	std::ifstream fInput(input);
	string currentLine;
	while (std::getline(fInput, currentLine))
	{
		m_CurrentCode.push_back(currentLine);
	}
}

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
		cout << rang::fg::red << "[ERROR] " << rang::fg::reset << "Invalid Code found!" << endl;
		for (auto error : m_ErrorStack)
		{
			cout << rang::fg::red << "[ERROR] " << rang::fg::reset << error << endl;
		}
		return false;
	}
	
	//c_Or();
	//c_And();
	c_Funcs();
	c_Allocs();
	c_EasyJmps();

	for (auto line : m_CurrentCode)
	{
		cout << line << endl;
	}

	return m_ErrorStack.size() <= 0;
}

void BonsaiCompiler::c_Allocs()
{
	static auto isAlloc = [&](string& line, string& a, string& b, string& h)
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
	static auto isZeroAlloc = [&](string& line, string& a)
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
			auto strLine = std::to_string(i);
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"start" + strLine + ": tst " + b,
				"jmp bNotNull" + strLine,
				"jmp bNull" + strLine,
				"bNotNull" + strLine + ": inc " + h,
				"inc " + a,
				"dec " + b,
				"jmp start" + strLine,
				"bNull" + strLine + ": tst " + h,
				"jmp restore" + strLine,
				"jmp resetHelper" + strLine,
				"restore" + strLine + ": dec " + h,
				"inc " + b,
				"jmp bNull" + strLine,
				"resetHelper" + strLine + ": [" + h + "=zero]",
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
			auto strLine = std::to_string(i);
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + a,
				"jmp notNull" + strLine,
				"jmp " + std::to_string(i + 1),
				"notNull" + strLine + ": dec " + a,
				"jmp " + std::to_string(allocAddy)
			};
			m_CurrentCode += func;
			c_Funcs();
		}
	}
}
void BonsaiCompiler::c_Funcs()
{
	static auto isFunc = [&](string& line, string& name)
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
	static auto isFuncJmp = [&](string& line, string& name)
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
void BonsaiCompiler::c_EasyJmps()
{
	static auto isEasyJmp = [&](string& line, string& to)
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