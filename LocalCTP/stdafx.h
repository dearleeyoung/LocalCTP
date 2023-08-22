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
#include <locale>
#include <codecvt>
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

// why use this class ?
// some complier may complain that class codecvt_byname is invalid because
// the ~codecvt_byname() is protected,
// so we use a new class to derive from codecvt_byname here.
template <class InternT, class ExternT, class State>
class CodecvtByname : public std::codecvt_byname<InternT, ExternT, State>
{
public:
    CodecvtByname(const char* id, size_t refs = 0)
        : std::codecvt_byname<InternT, ExternT, State>(id, refs)
    {
    }
    CodecvtByname(const std::string& id, size_t refs = 0)
        : std::codecvt_byname<InternT, ExternT, State>(id, refs)
    {
    }
    ~CodecvtByname() = default;
};

inline std::string gbk_to_utf8(const std::string& str)
{
#ifdef _WIN32
    const char* GBK_LOCALE_NAME = ".936";// GBK locale name in windows
#else
    const char* GBK_LOCALE_NAME = "zh_CN.gb18030";// GBK locale name in linux
#endif
    static std::wstring_convert<CodecvtByname<wchar_t, char, mbstate_t>> convert(
        new CodecvtByname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    std::wstring tmp_wstr = convert.from_bytes(str);

    static std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;
    return cv2.to_bytes(tmp_wstr);
}

inline std::string utf8_to_gbk(const std::string& str)
{
#ifdef _WIN32
    const char* GBK_LOCALE_NAME = ".936";// GBK locale name in windows
#else
    const char* GBK_LOCALE_NAME = "zh_CN.gb18030";// GBK locale name in linux
#endif
    static std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    std::wstring tmp_wstr = conv.from_bytes(str);

    static std::wstring_convert<CodecvtByname<wchar_t, char, mbstate_t>> convert(
        new CodecvtByname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    return convert.to_bytes(tmp_wstr);
}