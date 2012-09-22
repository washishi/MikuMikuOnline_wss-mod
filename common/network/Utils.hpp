//
// Utils.hpp
//
#pragma once

#include <string>
#include <tuple>
#include <boost/format.hpp>

#define NETWORK_UTILS_DELIMITOR (0x7e)

namespace network {
    namespace Utils {
        std::string Encode(const std::string&);
        std::string Decode(const std::string&);

        std::string ByteStuffingEncode(const std::string&);
        std::string ByteStuffingDecode(const std::string&);

        std::string Base64Encode(const std::string&);
        std::string Base64Decode(const std::string&);

        std::string LZ4Compress(const std::string& in);
        std::string LZ4Uncompress(const std::string& in, size_t size);

        std::string ToHexString(const std::string&);
		bool MatchWithWildcard(const std::string& pattern, const std::string& text);

        bool IsPrivateAddress(const std::string&);

        inline bool little_endian_check(int i)
        {
            return static_cast<bool>(*reinterpret_cast<char*>(&i));
        }

        inline bool little_endian()
        {
            return little_endian_check(1);
        }

        inline std::string ConvertEndian(const std::string& in)
        {
            if (little_endian()) {
                std::string out = in;
                std::reverse(out.begin(), out.end());
                return out;
            } else {
                return in;
            }
        }

        // Serialize
        template<class T>
        inline std::string GetSerializedValue(const T& t)
        {
            return ConvertEndian(std::string(reinterpret_cast<const char*>(&t), sizeof(t)));
        }

        template<>
        inline std::string GetSerializedValue(const std::string& t)
        {
            int size = t.size();
            return ConvertEndian(std::string(reinterpret_cast<const char*>(&size), sizeof(int))) +
                    std::string(t);
        }

        inline std::string Serialize() {
            return std::string();
        }

        template<class T1>
        inline std::string Serialize(const T1& t1)
        {
            return GetSerializedValue(t1);
        }

        template<class T1, class T2>
        inline std::string Serialize(const T1& t1, const T2& t2)
        {
            return GetSerializedValue(t1) + GetSerializedValue(t2);
        }

        template<class T1, class T2, class T3>
        inline std::string Serialize(const T1& t1, const T2& t2, const T3& t3)
        {
            return GetSerializedValue(t1) + GetSerializedValue(t2) + GetSerializedValue(t3);
        }

        template<class T1, class T2, class T3, class T4>
        inline std::string Serialize(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
        {
            return GetSerializedValue(t1) + GetSerializedValue(t2)
                + GetSerializedValue(t3) + GetSerializedValue(t4);
        }

        template<class T1, class T2, class T3, class T4, class T5>
        inline std::string Serialize(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
        {
            return GetSerializedValue(t1) + GetSerializedValue(t2)
                + GetSerializedValue(t3) + GetSerializedValue(t4)
                 + GetSerializedValue(t5);
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6>
        inline std::string Serialize(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
        {
            return GetSerializedValue(t1) + GetSerializedValue(t2)
                + GetSerializedValue(t3) + GetSerializedValue(t4)
                 + GetSerializedValue(t5) + GetSerializedValue(t6);
        }

        // Deserialize

        template<class First>
        inline First GetDeserializedValue(std::string& buffer)
        {
            const First value = *reinterpret_cast<const First*>(ConvertEndian(buffer.substr(0, sizeof(First))).data());
            buffer.erase(0, sizeof(First));
            return value;
        }

        template<>
        inline std::string GetDeserializedValue<std::string>(std::string& buffer)
        {
            int size = *reinterpret_cast<const int*>(ConvertEndian(buffer.substr(0, sizeof(int))).data());
            buffer.erase(0, sizeof(int));
            std::string data(buffer.data(), size);
            buffer.erase(0, size);
            return data;
        }

        template<class First>
        inline void GetDeserializedItems(std::string& buffer, First* first)
        {
            *first = GetDeserializedValue<First>(buffer);
        }

        template<class T1>
        inline size_t Deserialize(const std::string& data, T1 t1)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);

            return original_size - buffer.size();
        }

        template<class T1, class T2>
        inline size_t Deserialize(const std::string& data, T1 t1, T2 t2)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);
            GetDeserializedItems(buffer, t2);

            return original_size - buffer.size();
        }

        template<class T1, class T2, class T3>
        inline size_t Deserialize(const std::string& data, T1 t1, T2 t2, T3 t3)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);
            GetDeserializedItems(buffer, t2);
            GetDeserializedItems(buffer, t3);

            return original_size - buffer.size();
        }

        template<class T1, class T2, class T3, class T4>
        inline size_t Deserialize(const std::string& data, T1 t1, T2 t2, T3 t3, T4 t4)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);
            GetDeserializedItems(buffer, t2);
            GetDeserializedItems(buffer, t3);
            GetDeserializedItems(buffer, t4);

            return original_size - buffer.size();
        }

        template<class T1, class T2, class T3, class T4, class T5>
        inline size_t Deserialize(const std::string& data, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);
            GetDeserializedItems(buffer, t2);
            GetDeserializedItems(buffer, t3);
            GetDeserializedItems(buffer, t4);
            GetDeserializedItems(buffer, t5);

            return original_size - buffer.size();
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6>
        inline size_t Deserialize(const std::string& data, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
        {
            size_t original_size = data.size();
            std::string buffer(data);

            GetDeserializedItems(buffer, t1);
            GetDeserializedItems(buffer, t2);
            GetDeserializedItems(buffer, t3);
            GetDeserializedItems(buffer, t4);
            GetDeserializedItems(buffer, t5);
            GetDeserializedItems(buffer, t6);

            return original_size - buffer.size();
        }

    }
}
