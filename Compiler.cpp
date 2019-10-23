#include "Compiler.h"
#include <functional>
#include <fstream>
#include <iostream>
#include "rang.h"

using std::cout;
using std::endl;

regex easyBonsaiRegex(R"r(^(mov (\d+),\ *(\d+)\ *#\ *(\d+))|(mov (\d+)\ *,\ *NULL)|(goto (.+))|((.+):\ .*)|(jmp ([+-]\d+))|(and (\d+)\ *,\ *(\d+))|(or (\d+)\ *,\ *(\d+))|((.*):)|(mov (\d+),\ *(\d+)\ *)|(je (.+))|(jl (.+))|(jg (.+))|(cmp (\d+)\ *,\ *(\d+)\ *#\ *(\d+)\ *,\ *(\d+))|(cmp (\d+)\ *,\ *(\d+))|(jmp \.(.+))|(reg (.+)\ *,\ *(\d+))|(reg (.+))$)r");
regex addyRegex(R"r(^(tst (\d+))|(inc (\d+))|(dec (\d+))|(mov (\d+),\ *(\d+)\ *)|(mov (\d+)\ *,\ *NULL)|(and (\d+)\ *,\ *(\d+))|(or (\d+)\ *,\ *(\d+))|(cmp (\d+)\ *,\ *(\d+)\ *.*)$)r");
regex bonsaiValidationRegex(R"r(^(tst \d+)|(jmp \d+)|(inc \d+)|(dec \d+)|(hlt)$)r");
regex RAddyRegex(R"r((\w+)\ (.*))r");
regex customCallRegex(R"r(^((\w+) ((\d+\ *,*\ *)*))$)r");
regex customStartRegex(R"r(\.custom (.+))r");

map<MOD, int> _mods =
{
	{ IFUNC, 9 }, //! FuncName = 10
	{ FUNC, 19 }, //! FuncName = 21
	{ MOV, 1 },	  //...
	{ MOVS, 21 },
	{ MOVZ, 5},
	{ GOTO, 7},
	{ JMPTO, 38},
	{ RJMP, 11},
	{ AND, 13},
	{ OR, 16},
	{ CMP, 30},
	{ CMPS, 35},
	{ JE, 24},
	{ JL, 26},
	{ JG, 28},
	{ REG, 40},
	{ REGS, 43}
};

#ifndef BONSAI_WEB
BonsaiCompiler::BonsaiCompiler(string input, string output)
{
	this->m_OutPutFile = output;

	std::ifstream fInput(input);
	string currentLine;
	while (std::getline(fInput, currentLine))
	{
		if (currentLine.length() >= 1 && currentLine.substr(0, 1) == ";") continue;
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
		if (line.substr(0, 1) == ";") continue;
		m_CurrentCode.push_back(line);
	}
}
#endif

int BonsaiCompiler::maxAddy()
{
	int max = 0;
	for (auto element : m_CAddys)
	{
		if (element.second > max) max = element.second;
	}
	return max + 1;
}

void BonsaiCompiler::determineAddy()
{
	static const auto addyIndices = { 2,6,4,8,9,13,14,16,17,11, 19, 20 };
	if (!m_AutoHelper) return;
	for (int index = 0; m_CurrentCode.size() > index; index++)
	{
		auto line = m_CurrentCode[index];
		std::smatch res;
		if (regex_match(line, res, addyRegex))
		{
			for (auto index : addyIndices)
			{
				if (res[index].matched && std::find(m_DefaultAddys.begin(), m_DefaultAddys.end(), index) == m_DefaultAddys.end())
				{
					m_DefaultAddys.push_back(std::atoi(res[index].str().c_str()));
				}
			}
		}
	}
	int biggest = 0;
	for (auto addy : m_DefaultAddys)
	{
		if (addy > biggest) biggest = addy;
	}
	int biggest2 = 0;
	for (auto addy : m_CAddys)
	{
		if (addy.second > biggest2) biggest2 = addy.second;
	}

	if (biggest2 > biggest) biggest = biggest2;

	m_CAddys["ALLOC"] = ++biggest;
	m_CAddys["CMP1"] = ++biggest;
	m_CAddys["CMP2"] = ++biggest;

	m_CAddys["$R1"] = ++biggest;
	m_CAddys["$R2"] = ++biggest;
	m_CAddys["$R3"] = ++biggest;
	m_CAddys["$R4"] = ++biggest;
	m_CAddys["$R5"] = ++biggest;
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
	c_Funcs();
	c_Reg();
	determineAddy();
	c_Reg();

	c_Je();
	c_Jl();
	c_Jg();

	c_Lbls();
	c_Allocs();

	c_Or();
	c_And();
	c_Cmp();

	c_RelativeJmps();

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
			printf("%s", error.c_str());
		}
		return false;
	}

	return m_ErrorStack.size() <= 0;
}
void BonsaiCompiler::c_Or()
{
	static auto isOr = [](string& line, string& a, string& b)
	{
		static const auto _or = _mods[OR];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_or].matched)
			{
				a = res[_or + 1];
				b = res[_or + 2];
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
			c_Lbls();
		}
	}
}
void BonsaiCompiler::c_And()
{
	static auto isAnd = [](string& line, string& a, string& b)
	{
		static const auto _and = _mods[AND];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_and].matched)
			{
				a = res[_and];
				b = res[_and];
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
			c_Lbls();
		}
	}
}
void BonsaiCompiler::c_Allocs()
{
	static auto isAlloc = [](string& line, string& a, string& b, string& h)
	{
		static const auto _dalloc = _mods[MOV];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_dalloc].matched)
			{
				a = res[_dalloc + 1];
				b = res[_dalloc + 2];
				h = res[_dalloc + 3];
				return true;
			}
		}
		return false;
	};
	static auto isSmartAlloc = [this](string& line, string& a, string& b, string& h)
	{
		static const auto _salloc = _mods[MOVS];
		if (!m_AutoHelper) return false;
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_salloc].matched)
			{
				a = res[_salloc + 1];
				b = res[_salloc + 2];
				h = std::to_string(m_CAddys["ALLOC"]);
				return true;
			}
		}
		return false;
	};
	static auto isZeroAlloc = [](string& line, string& a)
	{
		static const auto zalloc = _mods[MOVZ];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[zalloc].matched)
			{
				a = res[zalloc + 1];
				return true;
			}
		}
		return false;
	};

	string a, b, h;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isAlloc(line, a, b, h) || isSmartAlloc(line, a, b, h))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"mov " + h + ", NULL",
				"mov " + a + ", NULL",
				"start: tst " + b,
				"goto bNotNull",
				"goto bNull",
				"bNotNull: inc " + h,
				"inc " + a,
				"dec " + b,
				"goto start",
				"bNull: tst " + h,
				"goto restore",
				"jmp " + std::to_string(i + 1),
				"restore: dec " + h,
				"inc " + b,
				"goto bNull",
			};
			m_CurrentCode += func;
			c_Lbls();
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
			c_Lbls();
		}
	}
}
void BonsaiCompiler::c_Lbls()
{
	m_FuncIndex.clear();

	static auto isFunc = [](string& line, string& name)
	{
		static const auto _func = _mods[IFUNC];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_func].matched)
			{
				name = res[_func + 1];
				return true;
			}
		}
		return false;
	};
	static auto isFuncT2 = [](string& line, string& name)
	{
		static const auto _func = _mods[FUNC];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_func].matched)
			{
				name = res[_func + 1];
				return true;
			}
		}
		return false;
	};
	static auto isFuncJmp = [](string& line, string& name)
	{
		static const auto _func = _mods[GOTO];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_func].matched)
			{
				name = res[_func + 1];
				return true;
			}
		}
		return false;
	};
	static auto isFuncJmp2 = [](string& line, string& name)
	{
		static const auto _func = _mods[JMPTO];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_func].matched)
			{
				name = res[_func + 1];
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
			m_FuncIndex.insert({ funcName, i + 1 });
		}
	}
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if ((isFuncJmp(line, funcName) || isFuncJmp2(line, funcName)) && m_FuncIndex.find(funcName) != m_FuncIndex.end())
		{
			m_CurrentCode[i] = "jmp " + std::to_string(m_FuncIndex.at(funcName));
		}
	}
}
void BonsaiCompiler::c_Je()
{
	static auto isJE = [](string& line, string& after)
	{
		static const auto _je = _mods[JE];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_je].matched)
			{
				after = res[_je + 1];
				return true;
			}
		}
		return false;
	};

	string a;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isJE(line, a))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + std::to_string(m_CAddys["CMP2"]),
				"jmp " + a,
				"jmp " + std::to_string(i + 1)
			};
			m_CurrentCode += func;
		}
	}
}
void BonsaiCompiler::c_Jg()
{
	static auto isJG = [](string& line, string& after)
	{
		static const auto _jg = _mods[JG];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_jg].matched)
			{
				after = res[_jg + 1];
				return true;
			}
		}
		return false;
	};

	string a;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isJG(line, a))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + std::to_string(m_CAddys["CMP1"]),
				"jmp " + a,
				"jmp " + std::to_string(i + 1)
			};
			m_CurrentCode += func;
		}
	}
}
void BonsaiCompiler::c_Jl()
{
	static auto isJL = [](string& line, string& after)
	{
		static const auto _jl = _mods[JL];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_jl].matched)
			{
				after = res[_jl + 1];
				return true;
			}
		}
		return false;
	};

	string a;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isJL(line, a))
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func =
			{
				"tst " + std::to_string(m_CAddys["CMP1"]),
				"jmp " + std::to_string(i + 1),
				"jmp " + a
			};
			m_CurrentCode += func;
		}
	}
}
void BonsaiCompiler::c_Cmp()
{
	static auto isCMP = [](string& line, string& a, string& b, string& h, string& h2)
	{
		static const auto _cmp = _mods[CMP];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_cmp].matched)
			{
				a = res[_cmp + 1];
				b = res[_cmp + 2];
				h = res[_cmp + 3];
				h2 = res[_cmp + 4];
				return true;
			}
		}
		return false;
	};
	static auto isCMPS = [this](string& line, string& a, string& b, string& h, string& h2)
	{
		static const auto _cmp = _mods[CMPS];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_cmp].matched)
			{
				a = res[_cmp + 1];
				b = res[_cmp + 2];
				h = std::to_string(m_CAddys["CMP1"]);
				h2 = std::to_string(m_CAddys["CMP2"]);
				return true;
			}
		}
		return false;
	};

	string a, b, h, h2;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isCMP(line, a, b, h, h2) || isCMPS(line, a, b, h, h2))
		{
			int allocAddy = m_CurrentCode.size();
			auto backAddy = std::to_string(i + 1);
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> newCode =
			{
				"mov " + h + "," + a,
				"mov " + h2 + "," + b,
				"start: tst " + a,
				"goto xNotNull",
				"goto xNull",
				"xNotNull: tst " + b,
				"goto yNotNull",
				"goto greater",
				"xNull: tst " + b,
				"goto less",
				"goto equal",
				"yNotNull: dec " + a,
				"dec " + b,
				"goto start",
				"less: mov " + a + "," + h,
				"mov " + b + "," + h2,
				"mov " + h + ", NULL",
				"mov " + h2 + ", NULL",
				"jmp " + backAddy,
				"greater: mov " + a + "," + h,
				"mov " + b + "," + h2,
				"mov " + h + ", NULL",
				"mov " + h2 + ", NULL",
				"inc " + h,
				"jmp " + backAddy,
				"equal: mov " + a + "," + h,
				"mov " + b + "," + h2,
				"mov " + h + ", NULL",
				"mov " + h2 + ", NULL",
				"inc " + h2,
				"jmp " + backAddy
			};
			m_CurrentCode += newCode;
			c_Lbls();
			c_Allocs();
		}
	}
}
void BonsaiCompiler::c_RelativeJmps()
{
	static auto isEasyJmp = [](string& line, string& to)
	{
		static const auto _rjmp = _mods[RJMP];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_rjmp].matched)
			{
				to = res[_rjmp + 1];
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
void BonsaiCompiler::c_Reg()
{
	static auto replace = [](std::string & data, std::string toSearch, std::string replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
	};
	static auto isREG = [](string& line, string& name, string& representation)
	{
		static const auto _rreg = _mods[REG];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_rreg].matched)
			{
				name = "$" + res[_rreg + 1].str();
				representation = res[_rreg + 2];
				return true;
			}
		}
		return false;
	};
	static auto isREGS = [this](string& line, string& name, string& representation)
	{
		static const auto _rreg = _mods[REGS];
		std::smatch res;
		if (std::regex_match(line, res, easyBonsaiRegex))
		{
			if (res[_rreg].matched)
			{
				name = "$" + res[_rreg + 1].str();
				representation = std::to_string(maxAddy());
				return true;
			}
		}
		return false;
	};

	string name, rep;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isREG(line, name, rep) || isREGS(line, name, rep))
		{
			m_CAddys.insert({ name, std::stoi(rep) });
			m_CurrentCode[i] = "jmp +1";
		}
	}

	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		std::smatch res;
		if (std::regex_match(line, res, RAddyRegex))
		{
			if (line.find("$") != string::npos)
			{
				for (auto addy : m_CAddys)
				{
					if (line.find(addy.first) != string::npos)
					{
						replace(line, addy.first, std::to_string(addy.second));
						m_CurrentCode[i] = line;
					}
				}
			}
		}
	}
}
void BonsaiCompiler::c_ClearFuncs()
{
	string functionName;
	vector<string> c_Code;
	bool currentlyInFunction = false;
	int start_I = 0;
	int end_I = 0;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		std::smatch res;
		auto line = m_CurrentCode[i];
		if (!currentlyInFunction && std::regex_match(line, res, customStartRegex))
		{
			start_I = i;
			currentlyInFunction = true;
			functionName = res[1];
			c_Code.clear();
			continue;
		}
		if (currentlyInFunction && line.substr(0, 4) == ".end")
		{
			currentlyInFunction = false;

			CustomInstruction custom;
			custom.name = functionName;
			custom.code = c_Code;

			m_Customs.insert({ functionName, custom });
			end_I = i;

			m_CurrentCode.erase(m_CurrentCode.begin() + start_I, m_CurrentCode.begin() + end_I + 1);
			c_ClearFuncs();
		}
		if (currentlyInFunction)
		{
			c_Code.push_back(line);
		}
	}
}
void BonsaiCompiler::c_Funcs()
{
	static auto replace = [](std::string & data, std::string toSearch, std::string replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
	};
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
	static auto isCALL = [this](string& line, string& name, string& params)
	{
		std::smatch res;
		if (std::regex_match(line, res, customCallRegex))
		{
			if (res[1].matched)
			{
				name = res[2];
				params = res[3];
				return true;
			}
		}
		return false;
	};

	c_ClearFuncs();

	string name, params;
	for (int i = 0; m_CurrentCode.size() > i; i++)
	{
		auto line = m_CurrentCode[i];
		if (isCALL(line, name, params) && m_Customs.find(name) != m_Customs.end())
		{
			int allocAddy = m_CurrentCode.size();
			m_CurrentCode[i] = "jmp " + std::to_string(allocAddy);
			vector<string> func;
			vector<string> parameter;
			auto splitted = split(params, ",");

			if (splitted.size() <= 1)
			{
				parameter = { params };
			}
			else
			{
				for (int x = 0; splitted.size() > x; x++)
				{
					parameter.push_back(splitted[x]);
				}
			}

			for (int x = 0; parameter.size() > x; x++)
			{
				func.push_back("mov " + ("$R" + std::to_string(x+1)) + ", " + parameter[x]);
			}

			for (auto line : m_Customs.at(name).code)
			{
				if (line.find("$.back+1") != string::npos)
					replace(line, "$.back+1", std::to_string(i + 2));
				if (line.find("$.back") != string::npos)
					replace(line, "$.back", std::to_string(i + 1));
				func.push_back(line);
			}
			//TODO: Move registers back
			//! Replace the jmp backs to a detour which moves all the registers back to the original variables.
			m_CurrentCode += func;
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
void BonsaiCompiler::setAutoHelperFlag(bool flag)
{
	this->m_AutoHelper = flag;
}