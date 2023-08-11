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
#include "LeeDateTime.h"

// 导出CTP的class需要的宏
#define ISLIB
#define LIB_TRADER_API_EXPORT