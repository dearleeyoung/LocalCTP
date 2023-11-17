#include "stdafx.h"

namespace localCTP
{
    using CovtByNm = CodecvtByname<wchar_t, char, mbstate_t>;

    CovtByNm* getCovtByNm()
    {
        CovtByNm* wcPtr(nullptr);
#ifdef _WIN32
        const char* GBK_LOCALE_NAME = ".936";// GBK locale name in windows
        wcPtr = new CovtByNm(GBK_LOCALE_NAME);
#else
        const char* GBK_LOCALE_NAME1 = "zh_CN.gbk";// GBK locale name in linux
        const char* GBK_LOCALE_NAME2 = "zh_CN.gb18030";// GB18030 locale name in linux
        const char* GBK_LOCALE_NAME3 = "zh_CN.gb2312";// GB2312 locale name in linux
        try
        {
            wcPtr = new CovtByNm(GBK_LOCALE_NAME1);
        }
        catch (...)
        {
            try
            {
                wcPtr = new CovtByNm(GBK_LOCALE_NAME2);
            }
            catch (...)
            {
                wcPtr = new CovtByNm(GBK_LOCALE_NAME3);
            }
        }
#endif
        return wcPtr;
    }


std::string gbk_to_utf8(const std::string& str)
{
    static std::wstring_convert<CovtByNm> convert(getCovtByNm());
    std::wstring tmp_wstr = convert.from_bytes(str);
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> cv2;

    return cv2.to_bytes(tmp_wstr);
}

std::string utf8_to_gbk(const std::string& str)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    std::wstring tmp_wstr = conv.from_bytes(str);
    static std::wstring_convert<CovtByNm> convert(getCovtByNm());

    return convert.to_bytes(tmp_wstr);
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string base64_decode(std::string const& encoded_string)
{
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

} // end namespace localCTP
