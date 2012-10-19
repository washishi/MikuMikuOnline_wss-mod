
#pragma once

#include <ostream>

#define TORADIAN(DEGREE)	(DEGREE*(DX_PI_F/180.0f))
#define TORADIAN_D(DEGREE)	(DEGREE*(DX_PI/180.0))
#define TORADIAN_LD(DEGREE)	((long double)DEGREE*((long double)DX_PI/180.0L))
#define TODEGREE(RADIAN)	(RADIAN*(180.0f/DX_PI_F))
#define TODEGREE_D(RADIAN)	(RADIAN*(180.0/DX_PI))

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

inline int GetColorHSV( float H, float S, float V )
{
	int hi ;
	float f, p, q, t ;
	float r, g, b ;
	int ir, ig, ib ;

	hi = ( int )( H / 60.0f ) ;
	hi = hi == 6 ? 5 : hi %= 6 ;
	f = H / 60.0f - ( float )hi ;
	p = V * ( 1.0f - S ) ;
	q = V * ( 1.0f - f * S ) ;
	t = V * ( 1.0f - ( 1.0f - f ) * S ) ;
	switch( hi )
	{
	case 0 : r = V ; g = t ; b = p ; break ;
	case 1 : r = q ; g = V ; b = p ; break ;
	case 2 : r = p ; g = V ; b = t ; break ;
	case 3 : r = p ; g = q ; b = V ; break ;
	case 4 : r = t ; g = p ; b = V ; break ;
	case 5 : r = V ; g = p ; b = q ; break ;
	}

	ir = ( int )( r * 255.0f ) ;
	     if( ir > 255 ) ir = 255 ;
	else if( ir <   0 ) ir =   0 ;

	ig = ( int )( g * 255.0f ) ;
	     if( ig > 255 ) ig = 255 ;
	else if( ig <   0 ) ig =   0 ;

	ib = ( int )( b * 255.0f ) ;
	     if( ib > 255 ) ib = 255 ;
	else if( ib <   0 ) ib =   0 ;

	return GetColor( ir, ig, ib ) ; 
}