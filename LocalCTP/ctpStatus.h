#pragma once
// coding by GB2312
#include "ThostFtdcUserApiDataType.h"
#include <string>

const TThostFtdcErrorMsgType ErrMsgUserInfoIsEmpty = "CTP:认证失败(UserID和BrokerID不能为空哦)";
const TThostFtdcErrorMsgType ErrMsgUserInfoNotSameAsLastTime = "CTP:认证失败(UserID和BrokerID需要和上次登录时相同哦)";
const TThostFtdcErrorMsgType ErrMsgNotAuth = "CTP:登录失败(没有认证)";
const TThostFtdcErrorMsgType ErrMsgUserInfoNotSameAsAuth = "CTP:登录失败(UserID和BrokerID需要和认证时相同哦)";
const TThostFtdcErrorMsgType ErrMsgDuplicateOrder = "CTP:重复的报单(OrderRef需比此前的最大值要大哦)";
const TThostFtdcErrorMsgType ErrMsg_INVALID_ORDERSIZE = "CTP:不合法的报单数量(需为正数)";
const TThostFtdcErrorMsgType ErrMsg_EXCHANGE_ID_IS_WRONG = "CTP:交易所代码错误";
const TThostFtdcErrorMsgType ErrMsg_INSTRUMENT_NOT_FOUND = "CTP:找不到合约";
const TThostFtdcErrorMsgType ErrMsg_BAD_PRICE_VALUE = "CTP:不支持的价格(需要是PriceTick的整数倍)";
const TThostFtdcErrorMsgType ErrMsg_NotExistOrder = "CTP:订单不存在哦";
const TThostFtdcErrorMsgType ErrMsg_AlreadyDoneOrder = "CTP:订单已结束无法再撤哦";
const TThostFtdcErrorMsgType ErrMsg_NotSupportModifyOrder = "CTP:订单只能撤单无法改单哦";
const TThostFtdcErrorMsgType ErrMsg_PRICETYPE_NOTSUPPORT_BYEXCHANGE = "CTP:交易所不支持的价格类型";
const TThostFtdcErrorMsgType ErrMsg_BAD_FIELD_ONLY_SPECULATION = "CTP:报单字段有误(本系统只支持投机类型)";
const TThostFtdcErrorMsgType ErrMsg_NotSupportContingentCondition = "CTP:不支持条件单哦(ContingentCondition)";
const TThostFtdcErrorMsgType ErrMsg_NotSupportTimeCondition = "CTP:不支持的有效期类型哦(TimeCondition)";
const TThostFtdcErrorMsgType ErrMsg_NoMarketData = "CTP:该合约没有行情数据";
const TThostFtdcErrorMsgType ErrMsg_INSTRUMENT_MARGINRATE_NOT_FOUND = "CTP:该合约没有保证金率数据";
const TThostFtdcErrorMsgType ERRMSG_INSTRUMENT_COMMISSIONRATE_NOT_FOUND = "CTP:该合约没有手续费率数据";
const TThostFtdcErrorMsgType ERRMSG_AVAILABLE_NOT_ENOUGH = "CTP:可用资金不足";
const TThostFtdcErrorMsgType ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH = "CTP:平仓时持仓不足,当前可平数量是";
const TThostFtdcErrorMsgType ERRMSG_AVAILABLE_TODAY_POSITION_NOT_ENOUGH = "CTP:平今时持仓不足,当前可平数量是";

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