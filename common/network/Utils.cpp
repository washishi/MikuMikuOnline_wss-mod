//
// Utils.cpp
//

#include "Utils.hpp"
#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>
#include <boost/asio/ip/address.hpp>

namespace network {
    namespace Utils {

        std::string Encode(const std::string& in)
        {
            return ByteStuffingEncode(in) + static_cast<char>(NETWORK_UTILS_DELIMITOR);
        }

        std::string Decode(const std::string& in)
        {
            return ByteStuffingDecode(in);
        }

        std::string ByteStuffingEncode(const std::string& in)
        {
            std::string out;

			for (auto it = in.begin(); it != in.end(); ++it) {
				const char& c = *it;
				if (c == 0x7e || c == 0x7d) {
                    out += 0x7d;
                    out += c ^ 0x20;
                } else {
                    out += c;
                }
			}

            return out;
        }

        std::string ByteStuffingDecode(const std::string& in)
        {
            std::string out;

            bool escape = false;

			for (auto it = in.begin(); it != in.end(); ++it) {
				const char& c = *it;
				if (escape) {
                    out += c ^ 0x20;
                    escape = false;
                } else if (!(escape = (c == 0x7d))) {
                    out += c;
                }
			}

            return out;
        }

        std::string ToHexString(const std::string& in)
        {
            std::string out;

			for (auto it = in.begin(); it != in.end(); ++it) {
				const char& c = *it;
				out += (boost::format("%X ") % (int)c).str();
			}

            return out;
        }

        bool IsPrivateAddress(const std::string& in)
        {
            typedef boost::asio::ip::address address;
            static auto private_a_from = address::from_string("10.0.0.0");
            static auto private_a_to   = address::from_string("10.255.255.255");
            static auto private_b_from = address::from_string("172.16.0.0");
            static auto private_b_to   = address::from_string("172.31.255.255");
            static auto private_c_from = address::from_string("192.168.0.0");
            static auto private_c_to   = address::from_string("192.168.255.255");

            auto addr = address::from_string(in);
            return (private_c_from <= addr && addr <= private_c_to) ||
                   (private_b_from <= addr && addr <= private_b_to) ||
                   (private_a_from <= addr && addr <= private_a_to);
        }

        static const char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        static const char reverse_table[128] = {
           64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
           64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
           64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
           52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
           64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
           15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
           64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
           41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
        };

        std::string Base64Encode(const std::string &bindata)
        {
           if (bindata.empty())
              return "";

           using std::string;
           using std::numeric_limits;

           if (bindata.size() > (numeric_limits<string::size_type>::max() / 4u) * 3u) {
              throw std::length_error("Converting too large a string to base64.");
           }

           const std::size_t binlen = bindata.size();
           // Use = signs so the end is properly padded.
           string retval((((binlen + 2) / 3) * 4), '=');
           std::size_t outpos = 0;
           int bits_collected = 0;
           unsigned int accumulator = 0;
           const string::const_iterator binend = bindata.end();

           for (string::const_iterator i = bindata.begin(); i != binend; ++i) {
              accumulator = (accumulator << 8) | (*i & 0xffu);
              bits_collected += 8;
              while (bits_collected >= 6) {
                 bits_collected -= 6;
                 retval[outpos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
              }
           }
           if (bits_collected > 0) { // Any trailing bits that are missing.
              assert(bits_collected < 6);
              accumulator <<= 6 - bits_collected;
              retval[outpos++] = b64_table[accumulator & 0x3fu];
           }
           assert(outpos >= (retval.size() - 2));
           assert(outpos <= retval.size());
           return retval;
        }

        std::string Base64Decode(const ::std::string &ascdata)
        {
           if (ascdata.empty())
               return "";

           using std::string;
           string retval;
           const string::const_iterator last = ascdata.end();
           int bits_collected = 0;
           unsigned int accumulator = 0;

           for (string::const_iterator i = ascdata.begin(); i != last; ++i) {
              const int c = *i;
              if (::std::isspace(c) || c == '=') {
                 // Skip whitespace and padding. Be liberal in what you accept.
                 continue;
              }
              if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
                 throw ::std::invalid_argument("This contains characters not legal in a base64 encoded string.");
              }
              accumulator = (accumulator << 6) | reverse_table[c];
              bits_collected += 6;
              if (bits_collected >= 8) {
                 bits_collected -= 8;
                 retval += (char)((accumulator >> bits_collected) & 0xffu);
              }
           }
           return retval;
        }

        std::string SnappyCompress(const std::string& in)
        {
			// TODO: VSに移行するため圧縮機能は無効
            return in;
        }

        std::string SnappyUncompress(const std::string& in)
        {
			// TODO: VSに移行するため圧縮機能は無効
            return in;
        }

    }
}
