#!/usr/bin/python3
#本脚本文件用于生成封装CTP相关类的C++代码. 生成的代码在 ./LocalCTP/auto_generated_code/ 目录中.
#在切换CTP头文件(./LocalCTP/current/ 目录中的文件) 后, 请执行此脚本以自动生成对应CTP版本的C++代码.
#使用方法: python3 ParseCTPHeaders.py
#
#使用后会生成3个文件:
#   CTPApiHugeMacro.h : 包含本系统不支持的CTP的API接口的函数默认实现的宏 UNSUPPORTED_CTP_API_FUNC. (本系统支持的API接口见下面的 exclusiveApiFunctions )
#   CTPSQLWrapper.h 和 CTPSQLWrapper.cpp : 封装CTP的SQL管理类的C++代码文件.


import copy

ctp_datatype_path = "../LocalCTP/ctp_file/current/ThostFtdcUserApiDataType.h"
ctp_struct_path = "../LocalCTP/ctp_file/current/ThostFtdcUserApiStruct.h"
ctp_api_path = "../LocalCTP/ctp_file/current/ThostFtdcTraderApi.h"
output_path = "../LocalCTP/auto_generated_code/CTPSQLWrapper.h"
output2_path = "../LocalCTP/auto_generated_code/CTPSQLWrapper.cpp"
output_api_path = "../LocalCTP/auto_generated_code/CTPApiHugeMacro.h"

needConvertMemberNames = ['StatusMsg','ErrorMsg'] #需要转换编码的成员变量
predefinedTableKey = {
    'CThostFtdcInvestorPositionField':['BrokerID','InvestorID','HedgeFlag','PositionDate','InstrumentID','PosiDirection'],
    'CThostFtdcInvestorPositionDetailField':['BrokerID','InvestorID','HedgeFlag','OpenDate','TradeID','InstrumentID','Direction'],
    'CThostFtdcOrderField':['BrokerID','InvestorID','TradingDay','FrontID','SessionID','OrderRef'],
    'CThostFtdcTradeField':['BrokerID','InvestorID','TradingDay','ExchangeID','TradeID','TradeType'],
    'CThostFtdcTradingAccountField':['BrokerID','AccountID'],
    'CThostFtdcInstrumentField':['InstrumentID'],
    'CThostFtdcInstrumentMarginRateField':['BrokerID','InvestorID','InstrumentID'],
    'CThostFtdcInstrumentCommissionRateField':['BrokerID','InvestorID','InstrumentID'],
}# 预先定义好主键的表. key:表名. value: 这张表的主键. 如果以后有想要保存的表并且知道主键,则可以在此处添加.

# 本系统已支持的API接口函数(比如登录请求ReqUserLogin), 无需为这些函数自动生成API重写的代码.
# 如果用户自行实现了其他接口,则可以在这个列表中添加接口函数名.
exclusiveApiFunctions = ['Release','Init','Join','GetTradingDay','RegisterFront','RegisterNameServer','RegisterFensUserInfo',
    'RegisterSpi','SubscribePrivateTopic','SubscribePublicTopic','ReqAuthenticate','ReqUserLogin','ReqUserLogout',
    'ReqUserPasswordUpdate','ReqTradingAccountPasswordUpdate','ReqOrderInsert','ReqParkedOrderInsert','ReqParkedOrderAction',
    'ReqOrderAction','ReqSettlementInfoConfirm','ReqRemoveParkedOrder','ReqRemoveParkedOrderAction',
    'ReqQryOrder','ReqQryTrade','ReqQryInvestorPosition','ReqQryTradingAccount','ReqQryInvestor','ReqQryInstrumentMarginRate',
    'ReqQryInstrumentCommissionRate','ReqQryExchange','ReqQryProduct','ReqQryInstrument','ReqQryDepthMarketData',
    'ReqQrySettlementInfo','ReqQryInvestorPositionDetail','ReqQrySettlementInfoConfirm','ReqQryClassifiedInstrument',
    'ReqQuoteInsert',
]


class CTPField:
    def __init__(self):
        self.fieldName = ''
        self.fieldType1 = ''
        self.fieldType2 = 0 #length of char array. if > 0, indicate that it is a str.
    def isStr(self):
        return (self.fieldType2 > 0)
    
class CTPClass:
    def __init__(self):
        self.clear()
    def clear(self):
        self.className = ''
        self.fields = [] # a list of tuple -- (CTPField, memberName)

def getSqlInsertField(ctpField : CTPField , memberName : str):
    if ctpField.fieldType1 != 'char':
        return "std::to_string(data."+memberName+")"
    elif ctpField.isStr():
        if memberName in needConvertMemberNames:
            return "gbk_to_utf8(" + "data."+memberName + ")"
        else:
            return "data."+memberName
    else: # single char, need to check whether is is '\0'
        return "(" + "data."+memberName + " == 0 ? '0' : data."+memberName+  ")" #TODO:如果该类型有枚举值,可以保存下来并在此处使用其中一个枚举值,而不用统一的'0'

def getSqlCreateTableField(ctpField : CTPField , memberName : str):
    section1 = "'" + memberName + "' "
    if ctpField.isStr():
        section2 = "VARCHAR(" + str(ctpField.fieldType2) + ")"
    elif ctpField.fieldType2 == "char":
        section2 = "VARCHAR(1)"
    elif ctpField.fieldType2 == "int" or ctpField.fieldType2 == "short":
        section2 = "INTEGER"
    elif ctpField.fieldType2 == "double" or ctpField.fieldType2 == "float":
        section2 = "DOUBLE"
    else:
        section2 = "INTEGER"
    section3 = " NOT NULL"
    return section1 + section2 + section3

fieldMap = {}
ctpClasses = {}



'''
读取到的内容:
typedef char A[13];
typedef char B;
typedef int C;
typedef double D;

struct X
{
    A a;
    B b;
    C c;
    D d;
};

需要生成的代码:
struct XWrapper
{
    static const std::string CREATE_TABLE_SQL;
    static const std::string SELECT_SQL;
    static const std::string INSERT_SQL_PREFIX;
    X data;

    XWrapper(const X& _data = {0}) :data(_data) {}
    XWrapper(const std::map<std::string, std::string>& rowValue) :data{0} { parseFromSqlValue(rowValue); }
    operator X() { return data; }

    void parseFromSqlValue(const std::map<std::string, std::string>& rowValue) {
        strncpy(data.a, rowValue.at("a").c_str(), sizeof(A));
        data.b = rowValue.at("B").empty() ? '0' : rowValue.at("B").c_str()[0];
        data.c = std::stoi(rowValue.at("C"));
        data.d = std::stod(rowValue.at("D"));
    }
    std::string generateInsertSql() const {
        const auto insertSqlBody = std::string() + "'" +
            data.a + "','" + data.b + "','" + std::to_string(data.c) + "','" + std::to_string(data.d)
            + "'";
        return INSERT_SQL_PREFIX + insertSqlBody + ");";
    }
};
const std::string XWrapper::CREATE_TABLE_SQL = "CREATE TABLE 'X' ('A' VARCHAR(13) NOT NULL, 'B' VARCHAR(1) NOT NULL, 'C' INTEGER NOT NULL, 'D' DOUBLE NOT NULL );";
const std::string XWrapper::SELECT_SQL = "SELECT * FROM 'X';";
const std::string XWrapper::INSERT_SQL_PREFIX = "REPLACE INTO 'X' VALUES (";

'''

# read CTP datatype from CTP header file
with open(ctp_datatype_path, 'r', encoding='GBK') as f:
    typedefPreFix = "typedef "
    definePreFix = "#define "#not used
    singleLine = f.readline()
    while len(singleLine) > 0:
        singleLine = singleLine.strip()
        #print(singleLine)
        if len(singleLine) == 0:
            singleLine = f.readline()
            continue
        if singleLine.startswith(typedefPreFix):
            typedefInfo = singleLine.split() #['typedef', 'char', 'TThostFtdcIdCardTypeType;']
            ctpField = CTPField()
            if '[' in typedefInfo[2]: # char array.  'typedef char TThostFtdcDateType[9];'
                ctpField.fieldName = typedefInfo[2].split('[')[0] # 'TThostFtdcDateType'
                ctpField.fieldType1 = typedefInfo[1] # 'char'
                ctpField.fieldType2 = int(typedefInfo[2].split('[')[1].split(']')[0]) # 9
            else: # 'typedef int TThostFtdcIPPortType;'
                ctpField.fieldName = typedefInfo[2].split(';')[0] # 'TThostFtdcIPPortType'
                ctpField.fieldType1 = typedefInfo[1] # 'int'
            fieldMap[ctpField.fieldName] = ctpField

        singleLine = f.readline()

# read CTP struct from CTP header file
with open(ctp_struct_path, 'r', encoding='GBK') as f:
    structBeginPreFix = "struct "
    structEndPreFix = "};"
    isInStruct = False
    ctpClass = CTPClass()
    singleLine = f.readline()
    while len(singleLine) > 0:
        singleLine = singleLine.strip()
        #print(singleLine)
        if len(singleLine) == 0:
            singleLine = f.readline()
            continue
        if not isInStruct:
            if singleLine.startswith(structBeginPreFix): # 'struct CThostFtdcReqUserLoginField'
                isInStruct = True
                ctpClass.className = singleLine.split()[1] # 'CThostFtdcReqUserLoginField'
        else:
            if singleLine.startswith(structEndPreFix):
                isInStruct = False
                ctpClasses[ctpClass.className] = copy.deepcopy(ctpClass)
                ctpClass.clear()
            else:
                if not singleLine.startswith('{') and not singleLine.startswith('///'):
                    fieldInfo = singleLine.split()
                    fieldName = fieldInfo[0]
                    if fieldName not in fieldMap:
                        print('field' + fieldName + 'of struct' + ctpClass.className + 'is not in fieldMap' )
                    else:
                        memberName = fieldInfo[1].split(';')[0]
                        ctpClass.fields.append((fieldMap[fieldName], memberName))

        singleLine = f.readline()

print(len(ctpClasses))

# write the SQL wrapper classes of CTP struct to the two cpp code files (.h and .cpp)
f2 = open(output2_path, 'w')
noticeStr = "// I am automatically generated by ParseCTPHeaders.py"
tapStr = ' ' * 4 # not use '\t' because its width is not certain
f2.write("#include \"CTPSQLWrapper.h\"\n")
f2.write(noticeStr + "\n")
f2.write("\n")
with open(output_path, 'w') as f:
    f.write("#pragma once\n")
    f.write(noticeStr + "\n")
    f.write("#include \"ThostFtdcUserApiStruct.h\"\n")
    f.write("#include <map>\n")
    f.write("#include <string>\n")
    f.write("#include \"../stdafx.h\"// gbk_to_utf8, utf8_to_gbk\n")
    f.write("\n")
    for className,ctpClass in ctpClasses.items():
        prefix = ""
        f.write(prefix + 'struct ' + className + 'Wrapper' + '\n')
        f.write(prefix + '{\n')
        prefix += tapStr
        f.write('''
    static const std::string CREATE_TABLE_SQL;
    static const std::string SELECT_SQL;
    static const std::string INSERT_SQL_PREFIX;
''')
        f.write(prefix + className +" data;\n")
        f.write("\n")
        f.write(prefix + className + "Wrapper(const "+className+"& _data = {0}) :data(_data) {}\n")
        f.write(prefix + className + "Wrapper(const std::map<std::string, std::string>& rowValue) :data{0} { parseFromSqlValue(rowValue); }\n")
        f.write(prefix + "operator "+className+"() { return data; }\n")
        f.write(prefix + "void parseFromSqlValue(const std::map<std::string, std::string>& rowValue) {\n")
        temp = prefix
        prefix += tapStr
        for field in ctpClass.fields:
            if field[0].isStr():
                if field[1] in needConvertMemberNames:
                    sourceStr = ", utf8_to_gbk(rowValue.at(\""+field[1]+"\")).c_str(),"
                else:
                    sourceStr = ", rowValue.at(\""+field[1]+"\").c_str(),"
                f.write(prefix + "strncpy(data."+field[1]+sourceStr+" sizeof("+field[0].fieldName+"));\n")
            elif field[0].fieldType1 == 'char':
                f.write(prefix + "data."+field[1]+" = rowValue.at(\""+field[1]+"\").empty() ? '0' : rowValue.at(\""+field[1]+"\")[0];\n")
            elif field[0].fieldType1 == 'int' or field[0].fieldType1 == 'short':
                f.write(prefix + "data."+field[1]+" = std::stoi(rowValue.at(\""+field[1]+"\"));\n")
            elif field[0].fieldType1 == 'double' or field[0].fieldType1 == 'float':
                f.write(prefix + "data."+field[1]+" = std::stod(rowValue.at(\""+field[1]+"\"));\n")
            else:
                print("Unknown type for field " + field[0].fieldName + " in class " + className)
        prefix = temp # resume the prefix
        f.write(prefix + "}\n")
        f.write(prefix + "std::string generateInsertSql() const {\n")
        prefix += tapStr
        f.write(prefix + "const auto insertSqlBody = std::string() + \"'\" +\n")
        f.write(prefix + tapStr + " + \"','\" + ".join(
            [ getSqlInsertField(fieldInfo,memberName) for (fieldInfo,memberName) in ctpClass.fields]) + "\n")
        f.write(prefix + tapStr + "+ \"'\";" + "\n")
        f.write(prefix + "return INSERT_SQL_PREFIX + insertSqlBody + \");\";\n")
        prefix = temp # resume the prefix
        f.write(prefix + "}\n")
        prefix = ""
        f.write(prefix + "};\n") # end struct
        # in CTPSQLWrapper.cpp file
        f2.write("const std::string "+className+"Wrapper::CREATE_TABLE_SQL = \"CREATE TABLE '"+className+"'(" +
            ", ".join([getSqlCreateTableField(fieldInfo,memberName) for (fieldInfo,memberName) in ctpClass.fields])
            + ("" if className not in predefinedTableKey else (", PRIMARY KEY(\\\""+  "\\\",\\\"".join(predefinedTableKey[className]) +"\\\")"))
            +");\";\n")
        f2.write("const std::string "+className+"Wrapper::SELECT_SQL = \"SELECT * FROM '"+className+"';\";\n")
        f2.write("const std::string "+className+"Wrapper::INSERT_SQL_PREFIX = \"REPLACE INTO '"+className+"' VALUES (\";\n")
        f2.write("\n")
        f.write(prefix + "\n")

f2.close()



class ApiFuncInfo:
    def __init__(self):
        self.funcComment = ''#函数注释(可能有多行,每行以" \"结尾)
        self.funcName = ''#函数名
        self.funcContent = ''#函数内容
    def __str__(self):
        return "ApiFuncInfo obj. funcName: " + self.funcName + " funcComment: " + self.funcComment + " funcContent: " + self.funcContent

apiFuncInfos = [] # a list of ApiFuncInfo

'''
#示例:
读取到的内容(CTP API函数):
	///请求查询产品报价汇率
	virtual int ReqQryProductExchRate(CThostFtdcQryProductExchRateField *pQryProductExchRate, int nRequestID) = 0;

	///请求查询产品组
	virtual int ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID) = 0;

生成的代码:
#define UNSUPPORTED_CTP_API_FUNC \
    /*///请求查询产品报价汇率*/ \
    virtual int ReqQryProductExchRate(CThostFtdcQryProductExchRateField *pQryProductExchRate, int nRequestID) override { return -1; } \
 \
    /*///请求查询产品组*/ \
    virtual int ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID) override { return -1; }

'''

# read API virtual functions from header file
with open(ctp_api_path, 'r', encoding='GBK') as f:
    classStartPreFix = "class TRADER_API_EXPORT CThostFtdcTraderApi"
    commentPreFix = "///"
    virtualFuncPreFix = "virtual "
    isInApiClass = False
    funcComment = ""
    macroPostFix = " \\\n"
    singleLine = f.readline()
    while len(singleLine) > 0:
        singleLine = singleLine.strip()
        #print(singleLine)
        if len(singleLine) == 0:
            singleLine = f.readline()
            funcComment = ""#读取到一个空行,则将API函数的注释清空
            continue
        if not isInApiClass:
            if singleLine.startswith(classStartPreFix):
                isInApiClass = True
            singleLine = f.readline()
            continue
        if singleLine.startswith(commentPreFix):
            # CTP的注释以///开头,放在宏里会导致最后的"\"无法被识别为续行符而续行,因此需要用/**/包起来套娃一层
            funcComment += tapStr + "/*" + singleLine + "*/" + macroPostFix
        elif singleLine.startswith(virtualFuncPreFix):
            funcContent = singleLine # "virtual int ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID) = 0;"
            apiFuncInfo = ApiFuncInfo()
            apiFuncInfo.funcName = funcContent.split()[2].split('(')[0] # "ReqQryProductGroup"
            if apiFuncInfo.funcName == 'char': # "virtual const char *GetTradingDay() = 0;"
                apiFuncInfo.funcName = "GetTradingDay"
            if funcContent.endswith("= 0;"):
                funcContent = funcContent[0:-4] + "override { return -1; }"
            apiFuncInfo.funcContent = funcContent
            apiFuncInfo.funcComment = funcComment
            #print(apiFuncInfo)
            apiFuncInfos.append(apiFuncInfo)
        singleLine = f.readline()

# write the api functions marcro to cpp code file, with encoding of utf-8 with BOM
with open(output_api_path, 'w', encoding='utf-8-sig') as f:
    f.write("#pragma once\n")
    f.write(noticeStr + "\n")
    #f.write("#pragma warning(disable: 4010)\n")
    f.write("\n")
    f.write("#ifndef UNSUPPORTED_CTP_API_FUN\n")
    f.write("#define UNSUPPORTED_CTP_API_FUNC \\\n")
    for i in range(len(apiFuncInfos)):
        apiFuncInfo = apiFuncInfos[i]
        if apiFuncInfo.funcName not in exclusiveApiFunctions:
            f.write(apiFuncInfo.funcComment)
            postFix = macroPostFix if i < len(apiFuncInfos)-1 else "\n"
            f.write(tapStr + apiFuncInfo.funcContent + postFix)
            f.write(postFix)
    f.write("#endif\n")