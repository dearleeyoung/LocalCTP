#pragma once
// coding by GB2312
#include "ThostFtdcUserApiDataType.h"
#include "stdafx.h"

namespace localCTP{

constexpr TThostFtdcErrorMsgType ErrMsgUserInfoIsEmpty = "CTP:认证失败(UserID和BrokerID不能为空哦)";
constexpr TThostFtdcErrorMsgType ErrMsgUserInfoNotSameAsLastTime = "CTP:认证失败(UserID和BrokerID需要和上次登录时相同哦)";
constexpr TThostFtdcErrorMsgType ErrMsgNotAuth = "CTP:登录失败(没有认证)";
constexpr TThostFtdcErrorMsgType ErrMsgUserInfoNotSameAsAuth = "CTP:登录失败(UserID和BrokerID需要和认证时相同哦)";
constexpr TThostFtdcErrorMsgType ErrMsgDuplicateOrder = "CTP:重复的报单(OrderRef需比此前的最大值要大哦)";
constexpr TThostFtdcErrorMsgType ErrMsg_INVALID_ORDERSIZE = "CTP:不合法的报单数量(需为正数)";
constexpr TThostFtdcErrorMsgType ErrMsg_EXCHANGE_ID_IS_WRONG = "CTP:交易所代码错误";
constexpr TThostFtdcErrorMsgType ErrMsg_INSTRUMENT_NOT_FOUND = "CTP:找不到合约";
constexpr TThostFtdcErrorMsgType ErrMsg_BAD_PRICE_VALUE = "CTP:不支持的价格(需要是PriceTick的整数倍):";
constexpr TThostFtdcErrorMsgType ErrMsg_NotExistOrder = "CTP:订单不存在哦";
constexpr TThostFtdcErrorMsgType ErrMsg_AlreadyDoneOrder = "CTP:订单已结束无法再撤哦";
constexpr TThostFtdcErrorMsgType ErrMsg_NotSupportModifyOrder = "CTP:订单只能撤单无法改单哦";
constexpr TThostFtdcErrorMsgType ErrMsg_PRICETYPE_NOTSUPPORT_BYEXCHANGE = "CTP:交易所不支持的价格类型(OrderPriceType)";
constexpr TThostFtdcErrorMsgType ErrMsg_BAD_FIELD_ONLY_SPECULATION = "CTP:报单字段有误(本系统只支持投机类型)(CombHedgeFlag)";
constexpr TThostFtdcErrorMsgType ErrMsg_NotSupportContingentCondition = "CTP:不支持特殊的条件单哦(ContingentCondition)";
constexpr TThostFtdcErrorMsgType ErrMsg_NotSupportTimeCondition = "CTP:不支持的有效期类型哦(TimeCondition)";
constexpr TThostFtdcErrorMsgType ErrMsg_NoMarketData = "CTP:该合约没有行情数据:";
constexpr TThostFtdcErrorMsgType ErrMsg_INSTRUMENT_MARGINRATE_NOT_FOUND = "CTP:该合约没有保证金率数据";
constexpr TThostFtdcErrorMsgType ERRMSG_INSTRUMENT_COMMISSIONRATE_NOT_FOUND = "CTP:该合约没有手续费率数据:";// not used
constexpr TThostFtdcErrorMsgType ERRMSG_AVAILABLE_NOT_ENOUGH = "CTP:可用资金不足";
constexpr TThostFtdcErrorMsgType ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH = "CTP:平仓时持仓不足,当前可平数量是:";
constexpr TThostFtdcErrorMsgType ERRMSG_AVAILABLE_TODAY_POSITION_NOT_ENOUGH = "CTP:平今时持仓不足,当前可平数量是:";
constexpr char CONDITIONAL_ORDER_SYSID_PREFIX[10] = "TJBD_";
constexpr char STR_YUE[10] = "月";

// 通过触发条件类型,判断是否是(符合本系统需求的)条件单
inline bool isConditionalType(TThostFtdcContingentConditionType	contingentCondition)
{
    return contingentCondition == THOST_FTDC_CC_LastPriceGreaterThanStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceGreaterEqualStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceLesserThanStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
}

inline std::string getStatusMsgByStatus(TThostFtdcOrderStatusType status)
{
    switch (status)
    {
        case THOST_FTDC_OST_AllTraded: // '0'
            return "全部成交";                
        case THOST_FTDC_OST_PartTradedQueueing: // '1'
            return "部分成交";
        case THOST_FTDC_OST_PartTradedNotQueueing: // '2'
            return "部分成交不在队列中";
        case THOST_FTDC_OST_NoTradeQueueing: // '3'
            return "未成交";
        case THOST_FTDC_OST_NoTradeNotQueueing: // '4'
            return "未成交不在队列中";
        case THOST_FTDC_OST_Canceled: // '5'
            return "已撤单";
        case THOST_FTDC_OST_Unknown: // 'a'
            return "未知";
        case THOST_FTDC_OST_NotTouched: // 'b'
            return "尚未触发";
        case THOST_FTDC_OST_Touched: // 'c'
            return "已触发";
        default:
            return "未知";
    }
}

inline bool isTradingDay(const CLeeDateTime& date)
{
    const int dayOfWeek = date.GetDayOfWeek();
    if (dayOfWeek == 6 || dayOfWeek == 0)//TODO:不会判断长假假日
    {
        return false;
    }
    return true;
}

inline std::string getNextTradingDay(CLeeDateTime dt)
{
    do {
        dt += CLeeDateTimeSpan(1, 0, 0, 0);
    } while (!isTradingDay(dt));
    return dt.Format("%Y%m%d");
}

inline std::string get_direction_name(const std::string& direction)
{
    static const std::map<std::string, std::string> direction_name_map{
        {std::string(1, THOST_FTDC_D_Buy), "买   "},
        {std::string(1, THOST_FTDC_D_Sell), "   卖"}
    };
    auto it_direction = direction_name_map.find(direction);
    if (it_direction != direction_name_map.end())
    {
        return it_direction->second;
    }
    else
    {
        return "未知";
    }
}
inline std::string get_direction_name(TThostFtdcDirectionType dir)
{
    return get_direction_name(std::string(1, dir));
}
inline std::string get_exchange_name(const std::string& ExchangeID)
{
    static const std::map<std::string, std::string> exchange_name_map{
        {"INE", "能源中心"},
        {"SHFE", "上期所"},
        {"CFFEX", "中金所"},
        {"DCE", "大商所"},
        {"CZCE", "郑商所"},
        {"GFEX", "广期所"},
        {"BSE", "北交所"},
        {"SSE", "上交所"},
        {"SHSE", "上交所"},
        {"SZSE", "深交所"}
    };
    auto it_exchange = exchange_name_map.find(ExchangeID);
    if (it_exchange != exchange_name_map.end())
    {
        return it_exchange->second;
    }
    else
    {
        return "未知";
    }
}
inline std::string get_open_close_name(const std::string& open_or_close)
{
    static const std::map<std::string, std::string> open_or_close_name_map{
        {std::string(1, THOST_FTDC_OF_Open), "开"},
        {std::string(1, THOST_FTDC_OF_Close), "  平"},
        {std::string(1, THOST_FTDC_OF_CloseToday), "平今"},
        {std::string(1, THOST_FTDC_OF_CloseYesterday), "平昨"},
        {std::string(1, THOST_FTDC_OF_ForceClose), "强平"},
        {std::string(1, THOST_FTDC_OF_ForceOff), "强减"},
        {std::string(1, THOST_FTDC_OF_LocalForceClose), "本地强平"}
    };
    auto it_open_or_close = open_or_close_name_map.find(open_or_close);
    if (it_open_or_close != open_or_close_name_map.end())
    {
        return it_open_or_close->second;
    }
    else
    {
        return "未知";
    }
}
inline std::string get_open_close_name(TThostFtdcOffsetFlagType open_or_close)
{
    return get_open_close_name(std::string(1, open_or_close));
}
//判断是否是期权品种
inline bool isOptions(TThostFtdcProductClassType ProductClass)
{
    return ProductClass == THOST_FTDC_PC_Options ||
        ProductClass == THOST_FTDC_PC_SpotOption;
}

///平仓明细
struct CloseDetail
{
    ///经纪公司代码
    TThostFtdcBrokerIDType BrokerID;
    ///投资者代码
    TThostFtdcInvestorIDType InvestorID;
    ///交易所代码
    TThostFtdcExchangeIDType ExchangeID;
    ///合约代码
    TThostFtdcInstrumentIDType InstrumentID;
    ///开仓日期(交易日)
    TThostFtdcDateType OpenDate;
    ///开仓价格
    TThostFtdcPriceType OpenPrice;
    ///开仓成交编号
    TThostFtdcTradeIDType OpenTradeID;
    ///平仓日期(交易日)
    TThostFtdcDateType CloseDate;
    ///平仓时间
    TThostFtdcTimeType CloseTime;
    ///平仓成交价格
    TThostFtdcPriceType ClosePrice;
    ///平仓成交编号
    TThostFtdcTradeIDType CloseTradeID;
    ///平仓手数
    TThostFtdcVolumeType CloseVolume;
    ///平仓成交的买卖方向
    TThostFtdcDirectionType Direction;
    ///昨结算价
    TThostFtdcPriceType PreSettlementPrice;
    ///平仓盈亏
    TThostFtdcMoneyType CloseProfit;
    ///实际平仓类型(平今or平昨)
    TThostFtdcOffsetFlagType CloseFlag;
    ///权利金收支
    TThostFtdcMoneyType CashIn;
};

///结算单
struct SettlementData
{
    ///经纪公司代码
    TThostFtdcBrokerIDType BrokerID;
    ///投资者代码
    TThostFtdcInvestorIDType InvestorID;
    ///结算单内容
    std::string SettlementContent;
    ///结算单日期(交易日)
    TThostFtdcDateType TradingDay;
    ///确认日期
    TThostFtdcDateType ConfirmDay;
    ///确认时间
    TThostFtdcTimeType ConfirmTime;
};

const std::string SETTLEMENT_CONTEXT(
R"(settlement_header1=_                                            LocalCTP系统                                           
settlement_header2=_                                                                    制表时间 Creation Date：%8s
settlement_header3=_----------------------------------------------------------------------------------------------------
settlement_header4=_                             交易结算单(盯市) Settlement Statement(MTM)                             
settlement_header5=_客户号 Client ID：  %-16s客户名称 Client Name：%s
settlement_header6=_日期 Date：%8s
settlement_header7=_
settlement_header8=_
settlement_header9=_
settlement_header10=_
settlement_account_summary1=_                   资金状况  币种：人民币  Account Summary  Currency：CNY 
settlement_account_summary2=_----------------------------------------------------------------------------------------------------
settlement_account_summary3=_期初结存 Balance b/f：               %13.2f  基础保证金 Initial Margin：        %13.2f
settlement_account_summary4=_出 入 金 Deposit/Withdrawal：        %13.2f  期末结存 Balance c/f：             %13.2f
settlement_account_summary5=_平仓盈亏 Realized P/L：              %13.2f  质 押 金 Pledge Amount：           %13.2f
settlement_account_summary6=_持仓盯市盈亏 MTM P/L：               %13.2f  客户权益 Client Equity：：         %13.2f
settlement_account_summary7=_期权执行盈亏 Exercise P/L：          %13.2f  货币质押保证金占用 FX Pledge Occ.：%13.2f
settlement_account_summary8=_手 续 费 Commission：                %13.2f  保证金占用 Margin Occupied：       %13.2f
settlement_account_summary9=_行权手续费 Exercise Fee：            %13.2f  交割保证金 Delivery Margin：       %13.2f
settlement_account_summary10=_交割手续费 Delivery Fee：            %13.2f  多头期权市值 Market value(long)：  %13.2f
settlement_account_summary11=_货币质入 New FX Pledge：             %13.2f  空头期权市值 Market value(short)： %13.2f
settlement_account_summary12=_货币质出 FX Redemption：             %13.2f  市值权益 Market value(equity)：    %13.2f
settlement_account_summary13=_质押变化金额 Chg in Pledge Amt：     %13.2f  可用资金 Fund Avail.：             %13.2f
settlement_account_summary14=_权利金收入 Premium received：        %13.2f  风 险 度 Risk Degree：            %13.2f%%
settlement_account_summary15=_权利金支出 Premium paid：            %13.2f  应追加资金 Margin Call：           %13.2f
settlement_account_summary16=_运行模式 Running Mode：              %s      结算时间 Margin Call：   %s
settlement_account_summary17=_
settlement_deposit_withdrawal_head1=_                                        出入金明细 Deposit/Withdrawal 
settlement_deposit_withdrawal_head2=_----------------------------------------------------------------------------------------------------------------
settlement_deposit_withdrawal_head3=_|发生日期|       出入金类型       |      入金      |      出金      |                   说明                   |
settlement_deposit_withdrawal_head4=_|  Date  |          Type          |    Deposit     |   Withdrawal   |                   Note                   |
settlement_deposit_withdrawal_head5=_----------------------------------------------------------------------------------------------------------------
settlement_deposit_withdrawal_single_record1=_|%-8s|出入金                  |%16.2f|%16.2f|%-42s|
settlement_deposit_withdrawal_end1=_----------------------------------------------------------------------------------------------------------------
settlement_deposit_withdrawal_end2=_|共%4d条|                        |%16.2f|%16.2f|                                          |
settlement_deposit_withdrawal_end3=_----------------------------------------------------------------------------------------------------------------
settlement_deposit_withdrawal_end4=_出入金---Deposit/Withdrawal     银期转账---Bank-Futures Transfer    银期换汇---Bank-Futures FX Exchange
settlement_deposit_withdrawal_end5=_
settlement_trade_head1=_                                                              成交记录 Transaction Record 
settlement_trade_head2=_---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_trade_head3=_|成交日期| 交易所 |       品种       |      合约      |买/卖|   投/保    |  成交价  | 手数 |   成交额   |       开平       |  手续费  |  平仓盈亏  |     权利金收支      |  成交序号  |
settlement_trade_head4=_|  Date  |Exchange|     Product      |   Instrument   | B/S |    S/H     |   Price  | Lots |  Turnover  |       O/C        |   Fee    |Realized P/L|Premium Received/Paid|  Trans.No. |
settlement_trade_head5=_---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_trade_single_record1=_|%-8s|%-8s|%-18s|%-16s|%-5s|投          |%10.3f|%6d|%12.2f|%-18s|%10.2f|%12.2f|%21.2f|%-12s|
settlement_trade_end1=_---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_trade_end2=_|共%4d条|        |                  |                      |            |          |%6d|%12.2f|                  |%10.2f|%12.2f|%21.2f|            |
settlement_trade_end3=_---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_trade_end4=_能源中心---INE  上期所---SHFE   中金所---CFFEX  大商所---DCE   郑商所---CZCE   广期所---GFEX
settlement_trade_end5=_买---Buy   卖---Sell
settlement_trade_end6=_投---Speculation  保---Hedge  套---Arbitrage 般---General
settlement_trade_end7=_开---Open 平---Close 平今---Close Today 强平---Forced Liquidation 平昨---Close Prev. 强减---Forced Reduction 本地强平---Local Forced Liquidation 
settlement_trade_end8=_
settlement_position_closed_head1=_                                                         平仓明细 Position Closed 
settlement_position_closed_head2=_----------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_closed_head3=_| 平仓日期 | 交易所 |       品种       |      合约      |开仓日期 |买/卖|   手数   |     开仓价    |     昨结算     |   成交价   |  平仓盈亏  |     权利金收支      |
settlement_position_closed_head4=_|Close Date|Exchange|      Product     |   Instrument   |Open Date| B/S |   Lots   |Pos. Open Price|   Prev. Sttl   |Trans. Price|Realized P/L|Premium Received/Paid|
settlement_position_closed_head5=_----------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_closed_single_record1=_|%-10s|%-8s|%-18s|%-16s|%-9s|%-5s|%10d|%15.3f|%16.3f|%12.3f|%12.2f|%21.2f|
settlement_position_closed_end1=_----------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_closed_end2=_|共%6d条|        |                  |                |         |     |%10d|               |                |            |%12.2f|%21.2f|
settlement_position_closed_end3=_----------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_closed_end4=_能源中心---INE  上期所---SHFE   中金所---CFFEX  大商所---DCE   郑商所---CZCE   广期所---GFEX
settlement_position_closed_end5=_买---Buy   卖---Sell 
settlement_position_closed_end6=_
settlement_position_detail_head1=_                                              持仓明细 Positions Detail
settlement_position_detail_head2=_-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_detail_head3=_| 交易所 |       品种       |      合约      |开仓日期 |   投/保    |买/卖|持仓量 |    开仓价     |     昨结算     |     结算价     |  浮动盈亏  |  盯市盈亏 |  保证金   |       期权市值       |
settlement_position_detail_head4=_|Exchange|     Product      |   Instrument   |Open Date|    S/H     | B/S |Positon|Pos. Open Price|   Prev. Sttl   |Settlement Price| Accum. P/L |  MTM P/L  |  Margin   | Market Value(Options)|
settlement_position_detail_head5=_-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_detail_single_record1=_|%-8s|%-18s|%-16s|%9s|投          |%-5s|%7d|%15.3f|%16.3f|%16.3f|%12.2f|%11.2f|%11.2f|                  0.00|
settlement_position_detail_end1=_-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_detail_end2=_|共%4d条|                  |                |         |            |     |%7d|               |                |                |%12.2f|%11.2f|%11.2f|                  0.00|
settlement_position_detail_end3=_-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_detail_end4=_能源中心---INE  上期所---SHFE   中金所---CFFEX  大商所---DCE   郑商所---CZCE   广期所---GFEX
settlement_position_detail_end5=_买---Buy   卖---Sell  
settlement_position_detail_end6=_投---Speculation  保---Hedge  套---Arbitrage 般---General
settlement_position_detail_end7=_
settlement_position_head1=_                                                         持仓汇总 Positions
settlement_position_head2=_------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_head3=_|       品种       |      合约      |    买持     |    买均价   |     卖持     |    卖均价    |  昨结算  |  今结算  |持仓盯市盈亏|  保证金占用   |  投/保     |   多头期权市值   |   空头期权市值    |
settlement_position_head4=_|      Product     |   Instrument   |  Long Pos.  |Avg Buy Price|  Short Pos.  |Avg Sell Price|Prev. Sttl|Sttl Today|  MTM P/L   |Margin Occupied|    S/H     |Market Value(Long)|Market Value(Short)|
settlement_position_head5=_------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_single_record1=_|%-18s|%-16s|%13d|%13.3f|%14d|%14.3f|%10.3f|%10.3f|%12.2f|%15.2f|投          |              0.00|               0.00|
settlement_position_end1=_------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_end2=_|共%6d条        |                |%13d|             |%14d|              |          |          |%12.2f|%15.2f|            |              0.00|               0.00|
settlement_position_end3=_------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
settlement_position_end4=_
settlement_position_end5=_
settlement_position_end6=_
settlement_position_end7=_
settlement_position_end8=_)");


} // end namespace localCTP
