// stdafx.h: 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 项目特定的包含文件
//

#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#else
#include <unistd.h>
#endif


// 在此处引用程序需要的其他标头
#include <vector>
#include <set>
#include <map>
#include <atomic>
#include <mutex>
#include <string>
#include <cstring>
#include <cmath>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cfloat>
#include "LeeDateTime.h"


// 导出CTP的class需要的宏
#define ISLIB
#define LIB_TRADER_API_EXPORT


constexpr double EPS = 1e-8;
inline bool EQ(double d1, double d2, double ep = EPS)
{
    return std::fabs(d1 - d2) < ep;
}

inline bool GE(double d1, double d2, double ep = EPS)
{
    return d1 >= (d2 - ep);
}

inline bool GT(double d1, double d2, double ep = EPS)
{
    return d1 > (d2 + ep);
}

inline bool NE(double d1, double d2, double ep = EPS)
{
    return !EQ(d1, d2, ep);
}

inline bool LE(double d1, double d2, double ep = EPS)
{
    return !GT(d1, d2, ep);
}

inline bool LT(double d1, double d2, double ep = EPS)
{
    return !GE(d1, d2, ep);
}

inline bool EQZ(double d, double ep = EPS)
{
    return std::fabs(d) < ep;
}

inline bool GEZ(double d, double ep = EPS)
{
    return d >= -ep;
}

inline bool GTZ(double d, double ep = EPS)
{
    return d > ep;
}

inline bool NEZ(double d, double ep = EPS)
{
    return !EQZ(d, ep);
}

inline bool LEZ(double d, double ep = EPS)
{
    return !GTZ(d, ep);
}

inline bool LTZ(double d, double ep = EPS)
{
    return !GEZ(d, ep);
}
