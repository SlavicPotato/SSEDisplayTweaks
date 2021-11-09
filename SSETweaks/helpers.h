#pragma once

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#ifdef UNICODE
#	define STD_STRING std::wstring
#	define STD_OFSTREAM std::wofstream
#	define STD_OSSTREAM std::wostringstream
#	define STD_ISSTREAM std::wistringstream
#	define STD_SSTREAM std::wstringstream
#	define STD_TOSTR std::to_wstring

#	define VSPRINTF _vsnwprintf_s
#	define SPRINTF _snwprintf_s
#	define STRCMP _wcsicmp
#	define FSOPEN _wfsopen
#	define MKDIR _wmkdir
#	define FPUTS fputws
#else
#	define STD_STRING std::string
#	define STD_OFSTREAM std::ofstream
#	define STD_OSSTREAM std::ostringstream
#	define STD_ISSTREAM std::istringstream
#	define STD_SSTREAM std::stringstream
#	define STD_TOSTR std::to_string

#	define VSPRINTF _vsnprintf_s
#	define SPRINTF _snprintf_s
#	define STRCMP _stricmp
#	define FSOPEN _fsopen
#	define MKDIR _mkdir
#	define FPUTS fputs
#endif

namespace FileHelpers
{
	using namespace std;

	SKMP_FORCEINLINE string GetPathFileNameA(const string& in)
	{
		string s(in);

		auto pos = s.rfind('\\');
		if (pos != string::npos)
		{
			return s.substr(pos + 1, string::npos);
		}

		pos = s.rfind('/');
		if (pos != string::npos)
		{
			return s.substr(pos + 1, string::npos);
		}

		return s;
	}

	SKMP_FORCEINLINE wstring GetPathFileNameW(const wstring& in)
	{
		wstring s(in);

		auto pos = s.rfind(L'\\');
		if (pos != wstring::npos)
		{
			return s.substr(pos + 1, wstring::npos);
		}

		pos = s.rfind(L'/');
		if (pos != wstring::npos)
		{
			return s.substr(pos + 1, wstring::npos);
		}

		return s;
	}
}
