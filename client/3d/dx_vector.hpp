
#pragma once

#include <ostream>

#define TORADIAN(DEGREE)	((DEGREE*PHI_F)/180.0f)

inline VECTOR operator +(const VECTOR& lhs, const VECTOR& rhs)
{
    return VAdd(lhs, rhs);
}

inline VECTOR operator -(const VECTOR& lhs, const VECTOR& rhs)
{
    return VSub(lhs, rhs);
}

inline VECTOR operator *(const VECTOR& lhs, float rhs)
{
    return VScale(lhs, rhs);
}

inline VECTOR operator *(float lhs, const VECTOR& rhs)
{
    return rhs * lhs;
}

inline float operator *(const VECTOR& lhs, const VECTOR& rhs)
{
    return VDot(lhs, rhs);
}

inline VECTOR& operator +=(VECTOR& lhs, const VECTOR& rhs)
{
    return (lhs = lhs + rhs);
}

inline VECTOR& operator -=(VECTOR& lhs, const VECTOR& rhs)
{
    return (lhs = lhs - rhs);
}

inline VECTOR& operator *=(VECTOR& lhs, float rhs)
{
    return (lhs = lhs * rhs);
}

inline VECTOR VAdjustLength(const VECTOR& lhs, float rhs)
{
    return lhs * (rhs / VSize(lhs));
}

inline bool operator ==(const VECTOR& lhs, const VECTOR& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator !=(const VECTOR& lhs, const VECTOR& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator <<(std::ostream& os, const VECTOR& v)
{
    os << "(" << v.x << "," << v.y << "," << v.z << ")";
    return os;
}
