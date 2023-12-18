#pragma once
#include <ctime>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__) || defined(__linux__)
#include <sys/time.h>
#endif

using std::string;

class CLeeDateTime;

//日期时间跨度类
class CLeeDateTimeSpan {
	friend class CLeeDateTime;//友元
public:
	CLeeDateTimeSpan() : m_span(0.0){ }
	CLeeDateTimeSpan(int days, int hours, int minutes, int seconds, int milliseconds = 0) 
		: m_span(days + hours / 24.0 + minutes / 60.0 / 24.0 
			+ seconds / 3600.0 / 24.0 + milliseconds / 1000.0 / 3600.0 / 24.0){ }
	CLeeDateTimeSpan(double span) : m_span(span){ }
	CLeeDateTimeSpan& operator=(const CLeeDateTimeSpan&rhs)  {
		if (this == &rhs) {
			return *this;
		}
		m_span = rhs.m_span;
		return *this;
	}
	operator double() const  { return m_span; }//类型转换函数,转换为double
	void SetDateTimeSpan(int days, int hours, int minutes, int seconds, int milliseconds = 0)  {
		m_span = days + hours / 24.0 + minutes / 60.0 / 24.0 
			+ seconds / 3600.0 / 24.0 + milliseconds / 1000.0 / 3600.0 / 24.0;
	}
	bool operator==(const CLeeDateTimeSpan& rhs) const {
		return m_span - rhs.m_span < THREE_MILLISECOND &&
			rhs.m_span - m_span < THREE_MILLISECOND;
	}
	bool operator!=(const CLeeDateTimeSpan& rhs) const { return !(*this == rhs); }
	bool operator>(const CLeeDateTimeSpan& rhs) const {
		return m_span - rhs.m_span >= THREE_MILLISECOND;
	}
	bool operator<(const CLeeDateTimeSpan& rhs) const {
		return  rhs.m_span - m_span >= THREE_MILLISECOND;
	}
	bool operator>=(const CLeeDateTimeSpan& rhs) const {
		return  !(*this < rhs);
	}
	bool operator<=(const CLeeDateTimeSpan& rhs) const {
		return  !(*this > rhs);
	}
	CLeeDateTimeSpan operator+(const CLeeDateTimeSpan& rhs) const {
		return CLeeDateTimeSpan(m_span + rhs.m_span);
	}
	CLeeDateTimeSpan operator-(const CLeeDateTimeSpan& rhs) const {
		return CLeeDateTimeSpan(m_span - rhs.m_span);
	}
	CLeeDateTimeSpan& operator+=(const CLeeDateTimeSpan& rhs) {
		m_span += rhs.m_span;
		return *this;
	}
	CLeeDateTimeSpan& operator-=(const CLeeDateTimeSpan& rhs) {
		m_span -= rhs.m_span;
		return *this;
	}
	double GetTotalDays() const  { return m_span; }//得到总天数
	double GetTotalHours() const  { return m_span * 24.0; }//得到总小时数
	double GetTotalMinutes() const  { return m_span * (24.0 * 60.0); }//得到总分钟数
	double GetTotalSeconds() const  { return m_span * (24.0 * 60.0 * 60.0); }//得到总秒数
	double GetTotalMilliseconds() const  { return m_span * (24.0 * 60.0 * 60.0 * 1000.0); }//得到总毫秒数

private:
	double m_span;//整数部分表示天数, 小数部分表示一天中的时间. 例如 2天3小时对应于2.125.
	const static double THREE_MILLISECOND;//三毫秒. 当两个CLeeDateTimeSpan之差在此之内时,则认为二者相等.
};

//日期时间类,windows中能表示1601/01/01以来的日期,linux中能表示的日期没有限制.
class CLeeDateTime {
public:
	CLeeDateTime() : m_dt(0.0) { }
	CLeeDateTime(double d) : m_dt(d) { }
	CLeeDateTime(time_t t) : m_dt(CLeeDateTime::time_t_to_double(t)) { }
	CLeeDateTime(const struct tm& s) ;
#ifdef _WIN32
	CLeeDateTime(const ::SYSTEMTIME& st) ;//会将毫秒数也引入
#endif
#if defined(__APPLE__) || defined(__linux__)
	CLeeDateTime(const timeval& st) ;//会将微秒数也引入,丢失精度变成毫秒.
#endif
	CLeeDateTime(int year, int month, int day, 
		int hour = 0, int minute = 0, int second = 0, int millisecond = 0) ;
	explicit CLeeDateTime(const string& str) ;
	CLeeDateTime(const CLeeDateTime& c) : m_dt(c.m_dt) { }//拷贝构造函数
	CLeeDateTime& operator=(const CLeeDateTime& rhs);//赋值函数

	string Format(const string& format = "%Y-%m-%d %H:%M:%S") const;//格式化输出时间(秒为近似值,四舍五入),参数格式请查阅strftime函数.
	string FormatWithMillisecond(const string& format = "%Y-%m-%d %H:%M:%S") const;//格式化输出时间, 得到带毫秒的字符串

	void SetDateTime(int year, int month, int day,
		int hour = 0, int minute = 0, int second = 0, int millisecond = 0) ;//设置日期时间
    void SetMiddleNight();
	void ParseDateTime(const string& str) ;//从字符串中解析日期时间

	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetMillisecond() const;//毫秒
	int GetDayOfWeek() const;//一周的第几天. 周日为0, 周六为6.
#ifdef _WIN32
	::SYSTEMTIME Get_SYSTEMTIME() const;//转为SYSTEMTIME
#endif
#if defined(__APPLE__) || defined(__linux__)
	::timeval Get_timeval() const;//转为timeval
#endif
    struct tm Get_tm() const;//转为struct tm(秒为近似值,四舍五入).
	struct tm Get_GMT_tm() const;//转为struct tm(秒为近似值,四舍五入).返回的是GMT时间的tm.
	time_t Get_time_t() const;//转为time_t(秒为近似值,四舍五入)

	bool IsLeapYear() const;//是否是闰年
	operator double() const { return m_dt; }//类型转换函数,转换为double
	
	bool operator==(const CLeeDateTime& rhs) const;
	bool operator!=(const CLeeDateTime& rhs) const;
	bool operator>(const CLeeDateTime& rhs) const;
	bool operator<(const CLeeDateTime& rhs) const;
	bool operator>=(const CLeeDateTime& rhs) const;
	bool operator<=(const CLeeDateTime& rhs) const;

	CLeeDateTime operator+(const CLeeDateTimeSpan& span) const ;
	CLeeDateTime& operator+=(const CLeeDateTimeSpan& span) ;
	CLeeDateTime operator-(const CLeeDateTimeSpan& span) const ;
	CLeeDateTime& operator-=(const CLeeDateTimeSpan& span) ;

	CLeeDateTimeSpan operator-(const CLeeDateTime& rhs) const ;//两个日期时间类相减

	static CLeeDateTime GetCurrentTime();//得到当前时间
    static CLeeDateTime now() { return GetCurrentTime(); }
	static bool IsLeapYear(int year);//某年是否是闰年
	static int GetDays(int year, int month);//得到某年某月的天数
	static int GetTimeZone();//得到当地时区						 
	static double time_t_to_double(time_t t);//将time_t类型时间转化为double类型
	static time_t double_to_time_t(double d);//将double类型时间转化为time_t类型(为近似值,四舍五入).若time_t为32位,则可能溢出.
    
private:
	double m_dt;//从1899-12-30 00:00:00开始的时间, 整数部分表示天数, 小数部分表示一天中的时间. 例如1899-12-31 06:00:00 对应为1.25. 
	const static double HALF_SECOND;//半秒
	const static double THREE_MILLISECOND;//三毫秒. 当两个CLeeDateTime之差在此之内时,则认为二者相等.
};
