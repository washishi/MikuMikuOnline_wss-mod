//
// FormatString.hpp
//

#pragma once
#include <boost/format.hpp>

namespace FormatString {

        template<class T1>
        std::string New(const std::string& format, const T1& t1) {
            return (boost::format(format) % t1).str();
        }

        template<class T1, class T2>
        std::string New(const std::string& format, const T1& t1, const T2& t2) {
            return (boost::format(format) % t1 % t2).str();
        }

        template<class T1, class T2, class T3>
        std::string New(const std::string& format, const T1& t1, const T2& t2, const T3& t3) {
            return (boost::format(format) % t1 % t2 % t3).str();
        }

        template<class T1, class T2, class T3, class T4>
        std::string New(const std::string& format, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
            return (boost::format(format) % t1 % t2 % t3 % t4).str();
        }
}
