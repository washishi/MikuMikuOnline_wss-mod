//
// Logger.hpp
//

#pragma once
#include <iostream>
#include "unicode.hpp"

#ifndef _WIN32
#define OutputDebugString(str) (str)
#endif

class Logger {
        // Singleton
    private:
        Logger() : ofs_("mmo_log.txt") {}
        Logger(const Logger& logger) {}
        virtual ~Logger() {}

    public:
        static void Info(const tstring& format) {
            getInstance().Log(_T("INFO: "), format);
        }

        template<class T1>
        static void Info(const tstring& format, const T1& t1) {
            getInstance().Log(_T("INFO: "), format, t1);
        }

        template<class T1, class T2>
        static void Info(const tstring& format, const T1& t1, const T2& t2) {
            getInstance().Log(_T("INFO: "), format, t1, t2);
        }

        template<class T1, class T2, class T3>
        static void Info(const tstring& format, const T1& t1, const T2& t2, const T3& t3) {
            getInstance().Log(_T("INFO: "), format, t1, t2, t3);
        }

        template<class T1, class T2, class T3, class T4>
        static void Info(const tstring& format, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
            getInstance().Log(_T("INFO: "), format, t1, t2, t3, t4);
        }


        static void Error(const tstring& format) {
            getInstance().Log(_T("ERROR: "), format);
        }

        template<class T1>
        static void Error(const tstring& format, const T1& t1) {
            getInstance().Log(_T("ERROR: "), format, t1);
        }

        template<class T1, class T2>
        static void Error(const tstring& format, const T1& t1, const T2& t2) {
            getInstance().Log(_T("ERROR: "), format, t1, t2);
        }

        template<class T1, class T2, class T3>
        static void Error(const tstring& format, const T1& t1, const T2& t2, const T3& t3) {
            getInstance().Log(_T("ERROR: "), format, t1, t2, t3);
        }

        template<class T1, class T2, class T3, class T4>
        static void Error(const tstring& format, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
            getInstance().Log(_T("ERROR: "), format, t1, t2, t3, t4);
        }


        static void Debug(const tstring& format) {
		#ifdef _DEBUG
            getInstance().Log(_T("DEBUG: "), format);
		#endif
        }

        template<class T1>
        static void Debug(const tstring& format, const T1& t1) {
		#ifdef _DEBUG
            getInstance().Log(_T("DEBUG: "), format, t1);
		#endif
        }

        template<class T1, class T2>
        static void Debug(const tstring& format, const T1& t1, const T2& t2) {
		#ifdef _DEBUG
            getInstance().Log(_T("DEBUG: "), format, t1, t2);
		#endif
        }

        template<class T1, class T2, class T3>
        static void Debug(const tstring& format, const T1& t1, const T2& t2, const T3& t3) {
		#ifdef _DEBUG
            getInstance().Log(_T("DEBUG: "), format, t1, t2, t3);
		#endif
        }

        template<class T1, class T2, class T3, class T4>
        static void Debug(const tstring& format, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
		#ifdef _DEBUG
            getInstance().Log(_T("DEBUG: "), format, t1, t2, t3, t4);
		#endif
        }


    private:
        typedef boost::basic_format<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> tformat;

        static Logger& getInstance() {
            static Logger instance;
            return instance;
        }

        void Log(const tstring& prefix, const tstring& format) {
            auto out = prefix + format + _T("\n");
            OutputDebugString(out.c_str());
			std::wcout << unicode::ToWString(out);
			ofs_ << unicode::ToString(out);
        }

        template<class T1>
        void Log(const tstring& prefix, const tstring& format, const T1& t1) {
            auto out = prefix + (tformat(format) % t1).str() + _T("\n");
            OutputDebugString(out.c_str());
			std::wcout << unicode::ToWString(out);
			ofs_ << unicode::ToString(out);
        }

        template<class T1, class T2>
        void Log(const tstring& prefix, const tstring& format, const T1& t1, const T2& t2) {
            auto out = prefix + (tformat(format) % t1 % t2).str() + _T("\n");
            OutputDebugString(out.c_str());
			std::wcout << unicode::ToWString(out);
			ofs_ << unicode::ToString(out);
        }

        template<class T1, class T2, class T3>
        void Log(const tstring& prefix, const tstring& format, const T1& t1, const T2& t2, const T3& t3) {
            auto out = prefix + (tformat(format) % t1 % t2 % t3).str() + _T("\n");
            OutputDebugString(out.c_str());
			std::wcout << unicode::ToWString(out);
			ofs_ << unicode::ToString(out);
        }

        template<class T1, class T2, class T3, class T4>
        void Log(const tstring& prefix, const tstring& format, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
            auto out = prefix + (tformat(format) % t1 % t2 % t3 % t4).str() + _T("\n");
            OutputDebugString(out.c_str());
			std::wcout << unicode::ToWString(out);
			ofs_ << unicode::ToString(out);
        }

		std::ofstream ofs_;
};
