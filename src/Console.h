#pragma once
#ifndef BONSAI_WEB
#include "belegpp/belegpp.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

using namespace beleg::helpers::print;
using namespace beleg::extensions::strings;
using namespace beleg::extensions::containers;

#pragma push_macro("ERROR")
#undef ERROR
namespace Console
{
	struct Color
	{
		std::uint16_t r, g, b;
		Color(std::uint8_t r, std::uint8_t g, std::uint8_t b) : r(r), g(g), b(b) { }
		void apply()
		{
			printf("\033[38;2;%i;%i;%im", r, g, b);
		}
		static void reset()
		{
			printf("\033[0;00m");
		}
	};
	inline void printFaded(std::string toPrint, Console::Color startColor, Console::Color endColor)
	{
		if (toPrint | contains("\n"))
		{
			auto splitted = toPrint | split("\n");
			for (auto line : splitted)
				printFaded(line, startColor, endColor);
		}
		else
		{
			auto sizeX = toPrint.length();
			if (sizeX > 0)
			{
				double rDiff = endColor.r - startColor.r;
				double gDiff = endColor.g - startColor.g;
				double bDiff = endColor.b - startColor.b;

				rDiff /= sizeX;
				gDiff /= sizeX;
				bDiff /= sizeX;

				Console::Color currentColor = Console::Color(startColor.r + rDiff, startColor.g + gDiff, startColor.b + bDiff);
				for (int i = 0; toPrint.length() > i; i++)
				{
					currentColor.apply();
					std::cout << toPrint[i];

					currentColor = Console::Color(currentColor.r + rDiff, currentColor.g + gDiff, currentColor.b + bDiff);
				}
			}
			Console::Color::reset();
		}
	}

	namespace internal
	{
#ifdef _WIN32
		/*
			Will enable colors in Windows-CMD.
		*/
		inline auto enableAnsiColors = []()
		{
			DWORD l_mode;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleMode(hStdout, &l_mode);
			SetConsoleMode(hStdout, l_mode |
				ENABLE_VIRTUAL_TERMINAL_PROCESSING |
				DISABLE_NEWLINE_AUTO_RETURN);

			return true;
		}();
#endif
		struct LineEnd { };
		inline const std::string getTime()
		{
#pragma warning(push)
#pragma warning(disable: 4996)

			time_t timer;
			char buffer[32];
			struct tm* tm_info;

			timer = time(NULL);
			tm_info = localtime(&timer);

			strftime(buffer, 32, "%H:%M:%S", tm_info);
			return buffer;
#pragma warning(pop)
		}
		enum LogLevel { INFO, DEBUG, ERROR };
		class Logger
		{
		private:
			LogLevel level;
			std::stringstream stream;

			void finish()
			{
				static Color errorCol(231, 76, 60);
				static Color timeCol(100, 100, 100);

				timeCol.apply();
				std::cout << "[" << getTime() << "] ";

				switch (level)
				{
				case INFO:
					printFaded("[EasyBonsai3] ", Console::Color(241, 196, 15), Console::Color(231, 76, 60));
					break;
				case DEBUG:
					timeCol.apply();
					std::cout << "[Debug] ";
					Color::reset();
					break;
				case ERROR:
					errorCol.apply();
					std::cout << "[Error] ";
					Color::reset();
				}

				std::cout << stream.str() << std::endl;
				stream.str("");
			}
		public:
			Logger(LogLevel level) : level(level)
			{ }
			template <typename T>
			Logger& operator<<(T what)
			{
				if constexpr (std::is_same<T, LineEnd>::value)
				{
					finish();
					return *this;
				}
				else
				{
					stream << what;
					return *this;
				}
			}
		};
	}

	inline internal::LineEnd endl;
	inline internal::Logger info(internal::LogLevel::INFO);
	inline internal::Logger error(internal::LogLevel::ERROR);
	inline internal::Logger debug(internal::LogLevel::DEBUG);
}
#pragma pop_macro("ERROR")
#endif