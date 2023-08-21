#include "LeeDateTime.h"
#include <sstream>
#include <cmath>

const double CLeeDateTimeSpan::THREE_MILLISECOND = 3 / 1000.0 / 3600.0 / 24.0;//三毫秒
const double CLeeDateTime::HALF_SECOND = 0.5 / 3600.0 / 24.0;//半秒
const double CLeeDateTime::THREE_MILLISECOND = 3 / 1000.0 / 3600.0 / 24.0;//三毫秒. 当两个CLeeDateTime之差在此之内时,则认为二者相等.

CLeeDateTime::CLeeDateTime(const struct tm& _tm)  
{
	int time_zone = CLeeDateTime::GetTimeZone();
	if (_tm.tm_year < 70 ||
		(_tm.tm_year == 70 && _tm.tm_mon == 0 && _tm.tm_mday == 1 && _tm.tm_hour < time_zone))//在1970/1/1(再加上时区)之前的struct tm日期, 通过mktime(&_tm)转换为time_t时, 得到-1
	{
		//每个月的天数. 非闰年
		const static int DaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (_tm.tm_mon < 0 || _tm.tm_mon > 11) {
			*this = CLeeDateTime();
			return;
		}
		int day_num_of_year = 0;
		for (int i = 0; i < 12; ++i) {
			if (i < _tm.tm_mon) {
				day_num_of_year += DaysOfMonth[i];
				if (i == 1 && CLeeDateTime::IsLeapYear(_tm.tm_year + 1900)) {
					day_num_of_year += 1;//闰年的二月
				}
			}
			else if (i == _tm.tm_mon) {
				day_num_of_year += _tm.tm_mday - 1;
				break;
			}
		}

		//确定当前年到1970年的总天数
		int rest_day_num_of_this_year_to_1970 = 0;
		for (int _year = _tm.tm_year + 1900; _year < 1970; ++_year)
		{
			if (CLeeDateTime::IsLeapYear(_year))
			{
				rest_day_num_of_this_year_to_1970 += 366;	//闰年
			}
			else
			{
				rest_day_num_of_this_year_to_1970 += 365;	//平年
			}
		}

		time_t total_second_num = rest_day_num_of_this_year_to_1970 * 24LL * 60LL * 60LL
			- day_num_of_year * 24LL * 60LL * 60LL
			- _tm.tm_hour * 60LL * 60LL
			- _tm.tm_min * 60LL
			- _tm.tm_sec
			;//若time_t为32位,则可能溢出. 使用24LL而不是24等.
		*this = CLeeDateTime(- total_second_num - static_cast<time_t>(time_zone * (60LL * 60LL)));//减去时区
	}
	else 
	{
		*this = CLeeDateTime(::mktime(const_cast<struct tm*>(&_tm)));
	}
}

CLeeDateTime::CLeeDateTime(int year, int month, int day, 
	int hour /*= 0*/, int minute /*= 0*/, int second /*= 0*/, int millisecond /*= 0*/) 
#ifdef _WIN32
	: CLeeDateTime(::SYSTEMTIME{
	static_cast<WORD>(year), static_cast<WORD>(month), 0, static_cast<WORD>(day),
	static_cast<WORD>(hour), static_cast<WORD>(minute), static_cast<WORD>(second),
	static_cast<WORD>(millisecond) })
#endif
{
#ifdef __linux__
	struct tm _tm = { 0 };
	_tm.tm_year = year - 1900;
	_tm.tm_mon = month - 1;
	_tm.tm_mday = day;
	_tm.tm_hour = hour;
	_tm.tm_min = minute;
	_tm.tm_sec = second;

	*this = CLeeDateTime(_tm) + CLeeDateTimeSpan(0, 0, 0, 0, millisecond);
#endif
}

#ifdef _WIN32
CLeeDateTime::CLeeDateTime(const ::SYSTEMTIME& st) 
{
	FILETIME fileTime1 = { 0 };//Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC时间).
	::SystemTimeToFileTime(&st, &fileTime1);
	long long ll = 0;
	::memcpy(&ll, &fileTime1, sizeof(::FILETIME));
	/*long long ll =
		((long long)fileTime1.dwHighDateTime << 32) | fileTime1.dwLowDateTime;*///比较慢
	m_dt = ll / (10.0 * 1000.0 * 1000.0 * 60.0 * 60.0 * 24.0) - 109205.0;//从1601/01/01到1899/12/30有109205天
}
#endif
#ifdef __linux__
CLeeDateTime::CLeeDateTime(const timeval& st) 
{
	*this = CLeeDateTime(st.tv_sec) + CLeeDateTimeSpan(0, 0, 0, 0, static_cast<int>(st.tv_usec) / 1000);
}
#endif

CLeeDateTime::CLeeDateTime(const string& str) 
{
	ParseDateTime(str);
}

string CLeeDateTime::Format(const string& format /*= "%Y-%m-%d %H:%M:%S"*/) const
{
	struct tm newtime = Get_tm();
	char tmpbuf[128] = { 0 };
	strftime(tmpbuf, 128, format.c_str(), &newtime);
	return string(tmpbuf);
}

//格式化输出时间, 得到带毫秒的字符串
string CLeeDateTime::FormatWithMillisecond(const string& format /*= "%Y-%m-%d %H:%M:%S"*/) const
{
#ifdef _WIN32
	long long ll = static_cast<long long>((m_dt + 109205.0)
		* 24.0 * 3600.0 * 1000.0 * 1000.0 * 10.0);//从1601/01/01到1899/12/30有109205天

	::FILETIME fileTime1 = { 0 };
	::memcpy(&fileTime1, &ll, sizeof(::FILETIME));
	::SYSTEMTIME sysTime1 = { 0 };
	::FileTimeToSystemTime(&fileTime1, &sysTime1);

	struct tm newtime {
		static_cast<int>(sysTime1.wSecond),
			static_cast<int>(sysTime1.wMinute),
			static_cast<int>(sysTime1.wHour),
			static_cast<int>(sysTime1.wDay),
			static_cast<int>(sysTime1.wMonth - 1),
			static_cast<int>(sysTime1.wYear - 1900),
			static_cast<int>(sysTime1.wDayOfWeek),
			0,
			0
	};//tm_yday和tm_isdst没有数据
	char tmpbuf[128] = { 0 };
	::strftime(tmpbuf, 128, format.c_str(), &newtime);

	char millisecond_buf[5] = { 0 };
	::sprintf(millisecond_buf, ".%03u", sysTime1.wMilliseconds);//毫秒的宽度是3, 不足3则用0填充

	return string(tmpbuf) + millisecond_buf;
#endif
#ifdef __linux__
	timeval tv = Get_timeval();
	time_t t = tv.tv_sec + CLeeDateTime::GetTimeZone() * (60LL * 60LL);
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	char tmpbuf[128] = { 0 };
	::strftime(tmpbuf, 128, format.c_str(), &_tm);

	char millisecond_buf[5] = { 0 };
	::sprintf(millisecond_buf, ".%03ld", tv.tv_usec / 1000);

	return string(tmpbuf) + millisecond_buf;
#endif
}

void CLeeDateTime::SetDateTime(int year, int month, int day, int hour /*= 0*/, int minute /*= 0*/, 
	int second /*= 0*/, int millisecond /*= 0*/) 
{
	*this = CLeeDateTime(year, month, day, hour, minute, second, millisecond);
}

void CLeeDateTime::ParseDateTime(const string& str) 
{
	std::istringstream iss(str);
	char symbol = 0;//用于读取间隔符号的字符
	int wYear(0), wMonth(0), wDay(0), wHour(0), wMinute(0), wSecond(0), wMilliseconds(0);
	iss >> wYear >> symbol;//先读取年份和第一个分隔符
	if (symbol == ':') {//如果字符串中没有日期,只有时间
		wYear = 1899;
		wMonth = 12;
		wDay = 30;
		iss.clear();
		iss.str(str);
		iss >> wHour >> symbol
			>> wMinute >> symbol
			>> wSecond >> symbol
			>> wMilliseconds;
	}
	else {
		iss >> wMonth >> symbol
			>> wDay
			>> wHour >> symbol
			>> wMinute >> symbol
			>> wSecond >> symbol
			>> wMilliseconds;
	}
	*this = CLeeDateTime(wYear, wMonth, wDay, wHour, wMinute, wSecond, wMilliseconds);
}

int CLeeDateTime::GetYear() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wYear;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_year + 1900;
#endif
}

int CLeeDateTime::GetMonth() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wMonth;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_mon + 1;
#endif
}

int CLeeDateTime::GetDay() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wDay;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_mday;
#endif
}

int CLeeDateTime::GetHour() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wHour;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_hour;
#endif
}

int CLeeDateTime::GetMinute() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wMinute;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_min;
#endif
}

int CLeeDateTime::GetSecond() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wSecond;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_sec;
#endif
}

int CLeeDateTime::GetMillisecond() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wMilliseconds;
#endif
#ifdef __linux__
	return static_cast<int>(Get_timeval().tv_usec) / 1000;
#endif
}

int CLeeDateTime::GetDayOfWeek() const
{
#ifdef _WIN32
	return Get_SYSTEMTIME().wDayOfWeek;
#endif
#ifdef __linux__
	double raw_time_t = (m_dt - 25569) * (24.0 * 60.0 * 60.0);
	time_t t = static_cast<time_t>(floor(raw_time_t));
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);
	return _tm.tm_wday;
#endif
}

#ifdef _WIN32
::SYSTEMTIME CLeeDateTime::Get_SYSTEMTIME() const
{
	long long ll = static_cast<long long>((m_dt + 109205.0)
		* 24.0 * 60.0 * 60.0 * 1000.0 * 1000.0 * 10.0);//从1601/01/01到1899/12/30有109205天
	::FILETIME fileTime1 = { 0 };
	::memcpy(&fileTime1, &ll, sizeof(::FILETIME));
	//fileTime1.dwHighDateTime = static_cast<DWORD>(ll >> 32);
	//fileTime1.dwLowDateTime = static_cast<DWORD>(ll | 0x00000000FFFFFFFF);//不准确
	::SYSTEMTIME sysTime1 = { 0 };
	::FileTimeToSystemTime(&fileTime1, &sysTime1);

	return sysTime1;
}
#endif
#ifdef __linux__
::timeval CLeeDateTime::Get_timeval() const {
	::timeval tv = { 0 };
	double raw_second_num = (m_dt - 25569) * (24.0 * 60.0 * 60.0)
        - CLeeDateTime::GetTimeZone() * (60.0 * 60.0);
	tv.tv_sec = static_cast<time_t>(floor(raw_second_num));
	tv.tv_usec = static_cast<__suseconds_t>((raw_second_num - static_cast<double>(tv.tv_sec)) * 1000000);
	return tv;
}
#endif

struct tm CLeeDateTime::Get_tm() const
{
#ifdef _WIN32
	::SYSTEMTIME sysTime1 =
        CLeeDateTime(m_dt + HALF_SECOND).Get_SYSTEMTIME();//因为tm结构体没有毫秒字段,所以加上半秒以四舍五入.
	struct tm newtime { static_cast<int>(sysTime1.wSecond),
		static_cast<int>(sysTime1.wMinute),
		static_cast<int>(sysTime1.wHour),
		static_cast<int>(sysTime1.wDay),
		static_cast<int>(sysTime1.wMonth - 1),
		static_cast<int>(sysTime1.wYear - 1900),
		static_cast<int>(sysTime1.wDayOfWeek),
		0,
		0 };//tm_yday和tm_isdst没有数据

	return newtime;
#endif
#ifdef __linux__
    time_t t = Get_time_t() + CLeeDateTime::GetTimeZone() * (60LL * 60LL);
	struct tm _tm = { 0 };
    ::gmtime_r(&t, &_tm);//日期时间小于1970-01-01时, localtime()返回值不准确, 故用gmtime()
    //linux中,localtime 不是线程安全的(返回的是内部static创建的指针), localtime_r 是线程安全的

	return _tm;
#endif
}

struct tm CLeeDateTime::Get_GMT_tm() const
{
#ifdef _WIN32
	::SYSTEMTIME sysTime1 =
		CLeeDateTime(m_dt - CLeeDateTime::GetTimeZone() / 24.0 + HALF_SECOND).Get_SYSTEMTIME();//因为tm结构体没有毫秒字段,所以加上半秒以四舍五入.
	struct tm newtime {
		static_cast<int>(sysTime1.wSecond),
			static_cast<int>(sysTime1.wMinute),
			static_cast<int>(sysTime1.wHour),
			static_cast<int>(sysTime1.wDay),
			static_cast<int>(sysTime1.wMonth - 1),
			static_cast<int>(sysTime1.wYear - 1900),
			static_cast<int>(sysTime1.wDayOfWeek),
			0,
			0
	};//tm_yday和tm_isdst没有数据

	return newtime;
#endif
#ifdef __linux__
	time_t t = Get_time_t();
	struct tm _tm = { 0 };
	::gmtime_r(&t, &_tm);//日期时间小于1970-01-01时, localtime()返回值不准确, 故用gmtime()
	//linux中,localtime 不是线程安全的(返回的是内部static创建的指针), localtime_r 是线程安全的

	return _tm;
#endif
}


time_t CLeeDateTime::Get_time_t() const
{
	return CLeeDateTime::double_to_time_t(m_dt);
}


CLeeDateTime CLeeDateTime::operator+(const CLeeDateTimeSpan& span) const 
{
	return CLeeDateTime(m_dt + span.m_span);
}

CLeeDateTime& CLeeDateTime::operator+=(const CLeeDateTimeSpan& span) 
{
	m_dt += span.m_span;
	return *this;
}

CLeeDateTime CLeeDateTime::operator-(const CLeeDateTimeSpan& span) const 
{
	return CLeeDateTime(m_dt - span.m_span);
}

CLeeDateTime& CLeeDateTime::operator-=(const CLeeDateTimeSpan& span) 
{
	m_dt -= span.m_span;
	return *this;
}

CLeeDateTimeSpan CLeeDateTime::operator-(const CLeeDateTime& rhs) const 
{
	return CLeeDateTimeSpan(m_dt - rhs.m_dt);
}

CLeeDateTime CLeeDateTime::GetCurrentTime(){
#ifdef _WIN32
	::SYSTEMTIME sysTime1 = { 0 };
	::GetLocalTime(&sysTime1);
	return CLeeDateTime(sysTime1);
#endif
#ifdef __linux__
	::timeval tv;
	::gettimeofday(&tv, nullptr);
	return CLeeDateTime(tv);
#endif
}

int CLeeDateTime::GetTimeZone() {
	static time_t tt0 = 0;
	static struct tm tm_test = { 0 };
	static bool bFirst = true;
	if (bFirst) {
        tm_test = *::localtime(&tt0);//linux中,localtime不是线程安全的(返回的是内部static创建的指针), 但由于立即解引用,所以没有问题
		bFirst = false;
	}
	return tm_test.tm_hour;
}

bool CLeeDateTime::IsLeapYear(int year)
{
	return (year % 4 == 0 && ((year % 400 == 0) || (year % 100 != 0)));
}

int CLeeDateTime::GetDays(int year, int month)
{
	int nDays = 0;
	switch (month) 
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			nDays = 31;
			break;
		case 2:
			nDays = IsLeapYear(year) ? 29 : 28;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			nDays = 30;
			break;
		default:
			break;
	}

	return nDays;
}

bool CLeeDateTime::IsLeapYear() const
{
	int year = GetYear();
	return CLeeDateTime::IsLeapYear(year);
}

CLeeDateTime& CLeeDateTime::operator=(const CLeeDateTime& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	m_dt = rhs.m_dt;
	return *this;
}

bool CLeeDateTime::operator>(const CLeeDateTime& rhs) const
{
	return m_dt - rhs.m_dt >= CLeeDateTime::THREE_MILLISECOND;
}

bool CLeeDateTime::operator<(const CLeeDateTime& rhs) const
{
	return rhs.m_dt - m_dt >= CLeeDateTime::THREE_MILLISECOND;
}

bool CLeeDateTime::operator>=(const CLeeDateTime& rhs) const
{
	return !(*this < rhs);
}

bool CLeeDateTime::operator<=(const CLeeDateTime& rhs) const
{
	return !(*this > rhs);
}

bool CLeeDateTime::operator==(const CLeeDateTime& rhs) const
{
	return m_dt - rhs.m_dt < CLeeDateTime::THREE_MILLISECOND &&
		rhs.m_dt - m_dt < CLeeDateTime::THREE_MILLISECOND;
}

bool CLeeDateTime::operator!=(const CLeeDateTime& rhs) const
{
	return !(*this == rhs);
}

double CLeeDateTime::time_t_to_double(time_t t) {
    return static_cast<double>(t) / (24.0 * 60.0 * 60.0) + 25569.0 + CLeeDateTime::GetTimeZone() / 24.0;//1899/12/30到1970/1/1之间有25569天
}

time_t CLeeDateTime::double_to_time_t(double d) {
    return static_cast<time_t>(floor((d - 25569) * (24.0 * 60.0 * 60.0)
        - CLeeDateTime::GetTimeZone() * (60.0 * 60.0) + 0.5));//+0.5是为了四舍五入,例如4.2秒+0.5,为4.7秒,转换为整数仍为4秒, 5.9秒+0.5,为6.4秒, 转换为整数仍为6秒. floor是为了dt为负数时向下取整(-1.2 -> -2).
}
