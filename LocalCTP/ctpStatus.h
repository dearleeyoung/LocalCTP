#pragma once
// coding by GB2312
#include "ThostFtdcUserApiDataType.h"
#include "stdafx.h"

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

// 通过触发条件类型,判断是否是(符合本系统需求的)条件单
inline bool isConditionalType(TThostFtdcContingentConditionType	contingentCondition)
{
    return contingentCondition == THOST_FTDC_CC_LastPriceGreaterThanStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceGreaterEqualStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceLesserThanStopPrice ||
        contingentCondition == THOST_FTDC_CC_LastPriceLesserEqualStopPrice;
}

// 判断是否满足成交条件
inline bool isMatchTrade(TThostFtdcDirectionType direction, double orderPrice, const CThostFtdcDepthMarketDataField& md)
{
    return ((direction == THOST_FTDC_D_Buy && GE(orderPrice, md.AskPrice1)) ||
        (direction == THOST_FTDC_D_Sell && LE(orderPrice, md.BidPrice1)));
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