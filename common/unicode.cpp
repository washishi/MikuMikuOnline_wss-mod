//
// unicode.cpp
//

#include "unicode.hpp"

namespace unicode {

#ifdef _WIN32

    std::string sjis2utf8(const std::string& in) {

        int wchar_size = ::MultiByteToWideChar(CP_ACP,0,in.data(),in.size(),nullptr,0);
		auto wchar_buf = new wchar_t [wchar_size];
        ::MultiByteToWideChar(CP_ACP,0,in.data(),in.size(),wchar_buf,wchar_size);

        int utf8_size = ::WideCharToMultiByte(CP_UTF8,0,wchar_buf,wchar_size,nullptr,0,nullptr,nullptr);
		auto utf8_buf = new char[utf8_size];
        ::WideCharToMultiByte(CP_UTF8,0,wchar_buf,wchar_size,utf8_buf,utf8_size,nullptr,nullptr);

		auto out = std::string(utf8_buf, utf8_size);
		delete[] wchar_buf;
		delete[] utf8_buf;

        return out;
    }

    std::string utf82sjis(const std::string& in) {

        int wchar_size = ::MultiByteToWideChar(CP_UTF8,0,in.data(),in.size(),nullptr,0);
		auto wchar_buf = new wchar_t[wchar_size];
        ::MultiByteToWideChar(CP_UTF8,0,in.data(),in.size(),wchar_buf,wchar_size);

        int sjis_size = ::WideCharToMultiByte(CP_ACP,0,wchar_buf,wchar_size,nullptr,0,nullptr,nullptr);
        auto sjis_buf = new char[sjis_size];
        ::WideCharToMultiByte(CP_ACP,0,wchar_buf,wchar_size,sjis_buf,sjis_size,nullptr,nullptr);

		auto out = std::string(sjis_buf, sjis_size);
		delete[] wchar_buf;
		delete[] sjis_buf;

        return out;
    }

    std::string ToString(const std::wstring& s)
    {
        int utf8_size = ::WideCharToMultiByte(CP_UTF8,0,s.data(),s.size(),nullptr,0,nullptr,nullptr);
        std::vector<char> utf8_buf(utf8_size);
        ::WideCharToMultiByte(CP_UTF8,0,s.data(),s.size(),utf8_buf.data(),utf8_size,nullptr,nullptr);

        return std::string(utf8_buf.data(), utf8_size);
    }

    std::wstring ToWString(const std::string& s)
    {
        int wchar_size = ::MultiByteToWideChar(CP_UTF8,0,s.data(),s.size(),nullptr,0);
        std::vector<wchar_t> wchar_buf(wchar_size);
        ::MultiByteToWideChar(CP_UTF8,0,s.data(),s.size(),wchar_buf.data(),wchar_size);

        return std::wstring(wchar_buf.data(), wchar_size);
    }

#endif

}
