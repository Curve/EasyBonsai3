#pragma once
#include <map>
#include <string>
#include <vector>
#include <regex>
using std::string;
using std::vector;
using std::regex;
using std::map;

extern regex easyBonsaiRegex;
extern regex bonsaiValidationRegex;

template <typename T>
std::vector<T>& operator +=(std::vector<T>& vector1, const std::vector<T>& vector2) {
	vector1.insert(vector1.end(), vector2.begin(), vector2.end());
	return vector1;
}

class BonsaiCompiler
{
public:
	BonsaiCompiler(string input, string output);
	BonsaiCompiler(string input, string output, bool hideLineNumbers);
private:
	string m_OutPutFile;
	bool m_HideLineNumbers;
	vector<string> m_ErrorStack;
	vector<string> m_CurrentCode;
	map<string, int> m_FuncIndex;
	//! ~~~~~
	//! Compile Hierarchy
	//! - High Level Implementations First (cuz they will use low level implementations)
	//! - "Low Level" Implementations Last 
	//! ~~~~~

	//! ~~~~~~~
	//! I know it may not be the best solution to iterate over the code over and over for each implementation but it should work best.
	//! ~~~~~~~
	void c_Or();
	void c_And();
	void c_Funcs();
	void c_Allocs(); //! Works
	void c_EasyJmps();

	//! ~ Important ~
	bool validateCode();
public:
	bool compile();
	void writeResults();
};