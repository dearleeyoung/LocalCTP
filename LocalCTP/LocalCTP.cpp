#include "stdafx.h"
#include "LocalTraderApi.h"
#include "Properties.h"
#include <iostream>
#define READ_CHAR_ARRAY_MEMBER(m) \
    { std::getline(i, temp, ','); \
      strncpy(instr.m, temp.c_str(), sizeof(instr.m)); }

#define READ_MEMBER(m) \
    { std::getline(i, temp, ','); \
      std::istringstream iss(temp); \
      iss >> instr.m; }

#define ADD_TO_TODAY_VALUE(fieldName) todayPos.fieldName += pos.fieldName;

namespace localCTP
{

std::istream& operator>>(std::istream& i, CThostFtdcInstrumentField& instr)
{
    ///合约代码
    std::string temp;
    ///合约代码
    READ_CHAR_ARRAY_MEMBER(InstrumentID)
        ///交易所代码
        READ_MEMBER(ExchangeID)
        ///合约名称
        READ_CHAR_ARRAY_MEMBER(InstrumentName)
        ///合约在交易所的代码
        READ_CHAR_ARRAY_MEMBER(ExchangeInstID)
        ///产品代码
        READ_MEMBER(ProductID)
        ///产品类型
        READ_MEMBER(ProductClass)
        ///交割年份
        READ_MEMBER(DeliveryYear)
        ///交割月
        READ_MEMBER(DeliveryMonth)
        ///市价单最大下单量
        READ_MEMBER(MaxMarketOrderVolume)
        ///市价单最小下单量
        READ_MEMBER(MinMarketOrderVolume)
        ///限价单最大下单量
        READ_MEMBER(MaxLimitOrderVolume)
        ///限价单最小下单量
        READ_MEMBER(MinLimitOrderVolume)
        ///合约数量乘数
        READ_MEMBER(VolumeMultiple)
        ///最小变动价位
        READ_MEMBER(PriceTick)
        ///创建日
        READ_MEMBER(CreateDate)
        ///上市日
        READ_MEMBER(OpenDate)
        ///到期日
        READ_MEMBER(ExpireDate)
        ///开始交割日
        READ_MEMBER(StartDelivDate)
        ///结束交割日
        READ_MEMBER(EndDelivDate)
        ///合约生命周期状态
        READ_MEMBER(InstLifePhase)
        ///当前是否交易
        READ_MEMBER(IsTrading)
        ///持仓类型
        READ_MEMBER(PositionType)
        ///持仓日期类型
        READ_MEMBER(PositionDateType)
        ///多头保证金率
        READ_MEMBER(LongMarginRatio)
        ///空头保证金率
        READ_MEMBER(ShortMarginRatio)
        ///是否使用大额单边保证金算法
        READ_MEMBER(MaxMarginSideAlgorithm)
        ///基础商品代码
        READ_MEMBER(UnderlyingInstrID)
        ///执行价
        READ_MEMBER(StrikePrice)
        ///期权类型
        READ_MEMBER(OptionsType)
        ///合约基础商品乘数
        READ_MEMBER(UnderlyingMultiple)
        ///组合类型
        READ_MEMBER(CombinationType)
        return i;
}

CThostFtdcInputOrderField CLocalTraderApi::OrderData::genrateInputOrderFromRtnOrder(
    const CThostFtdcOrderField& o)
{
    CThostFtdcInputOrderField inputOrder = { 0 };

    ///经纪公司代码
    strncpy(inputOrder.BrokerID, o.BrokerID, sizeof(inputOrder.BrokerID));
    ///投资者代码
    strncpy(inputOrder.InvestorID, o.InvestorID, sizeof(inputOrder.InvestorID));
    ///报单引用
    strncpy(inputOrder.OrderRef, o.OrderRef, sizeof(inputOrder.OrderRef));
    ///用户代码
    strncpy(inputOrder.UserID, o.UserID, sizeof(inputOrder.UserID));
    ///报单价格条件
    inputOrder.OrderPriceType = o.OrderPriceType;
    ///买卖方向
    inputOrder.Direction = o.Direction;
    ///组合开平标志
    strncpy(inputOrder.CombOffsetFlag, o.CombOffsetFlag, sizeof(inputOrder.CombOffsetFlag));
    ///组合投机套保标志
    strncpy(inputOrder.CombHedgeFlag, o.CombHedgeFlag, sizeof(inputOrder.CombHedgeFlag));
    ///价格
    inputOrder.LimitPrice = o.LimitPrice;
    ///数量
    inputOrder.VolumeTotalOriginal = o.VolumeTotalOriginal;
    ///有效期类型
    inputOrder.TimeCondition = o.TimeCondition;
    ///GTD日期
    strncpy(inputOrder.GTDDate, o.GTDDate, sizeof(inputOrder.GTDDate));
    ///成交量类型
    inputOrder.VolumeCondition = o.VolumeCondition;
    ///最小成交量
    inputOrder.MinVolume = o.MinVolume;
    ///触发条件
    inputOrder.ContingentCondition = o.ContingentCondition;
    ///止损价
    inputOrder.StopPrice = o.StopPrice;
    ///强平原因
    inputOrder.ForceCloseReason = o.ForceCloseReason;
    ///自动挂起标志
    inputOrder.IsAutoSuspend = o.IsAutoSuspend;
    ///业务单元
    strncpy(inputOrder.BusinessUnit, o.BusinessUnit, sizeof(inputOrder.BusinessUnit));
    ///请求编号
    inputOrder.RequestID = o.RequestID;
    ///用户强评标志
    inputOrder.UserForceClose = o.UserForceClose;
    ///互换单标志
    inputOrder.IsSwapOrder = o.IsSwapOrder;
    ///交易所代码
    strncpy(inputOrder.ExchangeID, o.ExchangeID, sizeof(inputOrder.ExchangeID));
    ///投资单元代码
    strncpy(inputOrder.InvestUnitID, o.InvestUnitID, sizeof(inputOrder.InvestUnitID));
    ///资金账号
    strncpy(inputOrder.AccountID, o.AccountID, sizeof(inputOrder.AccountID));
    ///币种代码
    strncpy(inputOrder.CurrencyID, o.CurrencyID, sizeof(inputOrder.CurrencyID));
    ///交易编码
    strncpy(inputOrder.ClientID, o.ClientID, sizeof(inputOrder.ClientID));
    ///Mac地址
    strncpy(inputOrder.MacAddress, o.MacAddress, sizeof(inputOrder.MacAddress));
    ///合约代码
    strncpy(inputOrder.InstrumentID, o.InstrumentID, sizeof(inputOrder.InstrumentID));
    ///IP地址
    strncpy(inputOrder.IPAddress, o.IPAddress, sizeof(inputOrder.IPAddress));

    return inputOrder;
}

CThostFtdcOrderField CLocalTraderApi::OrderData::genrateRtnOrderFromInputOrder(
    const CThostFtdcInputOrderField& InputOrder)
{
    CThostFtdcOrderField rtnOrder = { 0 };

    ///经纪公司代码
    strncpy(rtnOrder.BrokerID, InputOrder.BrokerID, sizeof(rtnOrder.BrokerID));
    ///投资者代码
    strncpy(rtnOrder.InvestorID, InputOrder.InvestorID, sizeof(rtnOrder.InvestorID));
    ///合约代码
    strncpy(rtnOrder.InstrumentID, InputOrder.InstrumentID, sizeof(rtnOrder.InstrumentID));
    ///报单引用
    strncpy(rtnOrder.OrderRef, InputOrder.OrderRef, sizeof(rtnOrder.OrderRef));
    ///用户代码
    strncpy(rtnOrder.UserID, InputOrder.UserID, sizeof(rtnOrder.UserID));
    ///报单价格条件
    rtnOrder.OrderPriceType = InputOrder.OrderPriceType;
    ///买卖方向
    rtnOrder.Direction = InputOrder.Direction;
    ///组合开平标志
    strncpy(rtnOrder.CombOffsetFlag, InputOrder.CombOffsetFlag, sizeof(rtnOrder.CombOffsetFlag));
    ///组合投机套保标志
    strncpy(rtnOrder.CombHedgeFlag, InputOrder.CombHedgeFlag, sizeof(rtnOrder.CombHedgeFlag));
    ///价格
    rtnOrder.LimitPrice = InputOrder.LimitPrice;
    ///数量
    rtnOrder.VolumeTotalOriginal = InputOrder.VolumeTotalOriginal;
    ///有效期类型
    rtnOrder.TimeCondition = InputOrder.TimeCondition;
    ///GTD日期
    strncpy(rtnOrder.GTDDate, "", sizeof(rtnOrder.GTDDate));
    ///成交量类型
    rtnOrder.VolumeCondition = InputOrder.VolumeCondition;
    ///最小成交量
    rtnOrder.MinVolume = InputOrder.MinVolume;
    ///触发条件
    rtnOrder.ContingentCondition = InputOrder.ContingentCondition;
    ///止损价
    rtnOrder.StopPrice = InputOrder.StopPrice;
    ///强平原因
    rtnOrder.ForceCloseReason = InputOrder.ForceCloseReason;
    ///自动挂起标志
    rtnOrder.IsAutoSuspend = InputOrder.IsAutoSuspend;
    ///业务单元
    strncpy(rtnOrder.BusinessUnit, "", sizeof(rtnOrder.BusinessUnit));
    ///请求编号
    rtnOrder.RequestID = InputOrder.RequestID;
    ///本地报单编号
    //也使用报单引用
    strncpy(rtnOrder.OrderLocalID, InputOrder.OrderRef, sizeof(rtnOrder.OrderLocalID));
    ///交易所代码
    strncpy(rtnOrder.ExchangeID, InputOrder.ExchangeID, sizeof(rtnOrder.ExchangeID));
    ///会员代码
    strncpy(rtnOrder.ParticipantID, "", sizeof(rtnOrder.ParticipantID));
    ///客户代码
    strncpy(rtnOrder.ClientID, "", sizeof(rtnOrder.ClientID));
    ///合约在交易所的代码
    strncpy(rtnOrder.ExchangeInstID, InputOrder.InstrumentID, sizeof(rtnOrder.ExchangeInstID));
    ///交易所交易员代码
    strncpy(rtnOrder.TraderID, "", sizeof(rtnOrder.TraderID));
    ///安装编号
    rtnOrder.InstallID = 0;
    ///报单提交状态
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    ///报单提示序号
    rtnOrder.NotifySequence = 0;
    ///交易日
    //strncpy(rtnOrder.TradingDay, api.GetTradingDay(), sizeof(rtnOrder.TradingDay));
    ///结算编号
    rtnOrder.SettlementID = 0;
    ///报单编号
    strncpy(rtnOrder.OrderSysID, "", sizeof(rtnOrder.OrderSysID));
    ///报单来源
    rtnOrder.OrderSource = THOST_FTDC_OSRC_Participant;
    ///报单状态
    rtnOrder.OrderStatus = THOST_FTDC_OST_Unknown;
    ///报单类型
    rtnOrder.OrderType = THOST_FTDC_ORDT_Normal;
    ///今成交数量
    rtnOrder.VolumeTraded = 0;
    ///剩余数量
    rtnOrder.VolumeTotal = rtnOrder.VolumeTotalOriginal;
    ///报单日期
    const CLeeDateTime now_time = CLocalTraderApi::getNowTime(); //CLeeDateTime::GetCurrentTime();
    strncpy(rtnOrder.InsertDate, now_time.Format("%Y%m%d").c_str(), sizeof(rtnOrder.InsertDate));
    ///委托时间
    strncpy(rtnOrder.InsertTime, now_time.Format("%H:%M:%S").c_str(), sizeof(rtnOrder.InsertTime));
    ///激活时间
    strncpy(rtnOrder.ActiveTime, "", sizeof(rtnOrder.ActiveTime));
    ///挂起时间
    strncpy(rtnOrder.SuspendTime, "", sizeof(rtnOrder.SuspendTime));
    ///最后修改时间
    strncpy(rtnOrder.UpdateTime, "", sizeof(rtnOrder.UpdateTime));
    ///撤销时间
    strncpy(rtnOrder.CancelTime, "", sizeof(rtnOrder.CancelTime));
    ///最后修改交易所交易员代码
    strncpy(rtnOrder.ActiveTraderID, "", sizeof(rtnOrder.ActiveTraderID));
    ///结算会员编号
    strncpy(rtnOrder.ClearingPartID, "", sizeof(rtnOrder.ClearingPartID));
    ///序号
    rtnOrder.SequenceNo = 0;
    ///前置编号
    //rtnOrder.FrontID = api.getFrontID();
    ///会话编号
    //rtnOrder.SessionID = api.getSessionID();
    ///用户端产品信息
    strncpy(rtnOrder.UserProductInfo, "", sizeof(rtnOrder.UserProductInfo));
    ///状态信息
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(), sizeof(rtnOrder.StatusMsg));
    ///用户强评标志
    rtnOrder.UserForceClose = InputOrder.UserForceClose;
    ///操作用户代码
    strncpy(rtnOrder.ActiveUserID, "", sizeof(rtnOrder.ActiveUserID));
    ///经纪公司报单编号
    rtnOrder.BrokerOrderSeq = 0;
    ///相关报单
    //strncpy(rtnOrder.RelativeOrderSysID, relativeOrderSysID.c_str(), sizeof(rtnOrder.RelativeOrderSysID));
    ///郑商所成交数量
    rtnOrder.ZCETotalTradedVolume = (strcmp(InputOrder.ExchangeID, "CZCE") == 0 ? rtnOrder.VolumeTraded : 0);
    ///互换单标志
    rtnOrder.IsSwapOrder = InputOrder.IsSwapOrder;
    ///营业部编号
    strncpy(rtnOrder.BranchID, "", sizeof(rtnOrder.BranchID));
    ///投资单元代码
    strncpy(rtnOrder.InvestUnitID, InputOrder.InvestUnitID, sizeof(rtnOrder.InvestUnitID));
    ///资金账号
    strncpy(rtnOrder.AccountID, InputOrder.AccountID, sizeof(rtnOrder.AccountID));
    ///币种代码
    strncpy(rtnOrder.CurrencyID, "CNY", sizeof(rtnOrder.CurrencyID));

    return rtnOrder;
}

bool CLocalTraderApi::OrderData::isDone() const
{
    return rtnOrder.OrderStatus != THOST_FTDC_OST_PartTradedQueueing &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_NoTradeQueueing &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_Unknown &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_NotTouched;
}

void CLocalTraderApi::OrderData::dealTestReqOrderInsertNormal(const CThostFtdcInputOrderField& InputOrder,
    const std::string& relativeOrderSysID)
{
    // "未知"状态
    strncpy(rtnOrder.TradingDay, api.GetTradingDay(), sizeof(rtnOrder.TradingDay));
    rtnOrder.FrontID = api.getFrontID();
    rtnOrder.SessionID = api.getSessionID();
    strncpy(rtnOrder.RelativeOrderSysID, relativeOrderSysID.c_str(), sizeof(rtnOrder.RelativeOrderSysID));

    sendRtnOrder();

    const auto OrderSysID = api.getNextOrderSysID(InputOrder.ExchangeID);
    if (isConditionalOrder)
    {
        // "未知"-> "未触发" 状态
        strncpy(rtnOrder.OrderSysID,
            (CONDITIONAL_ORDER_SYSID_PREFIX + std::to_string(OrderSysID)).c_str(),
            sizeof(rtnOrder.OrderSysID));
        rtnOrder.OrderStatus = THOST_FTDC_OST_NotTouched;
    }
    else
    {
        // "未知"-> "未成交" 状态 
        strncpy(rtnOrder.OrderSysID, std::to_string(OrderSysID).c_str(),
            sizeof(rtnOrder.OrderSysID));
        rtnOrder.OrderStatus = THOST_FTDC_OST_NoTradeQueueing; 
    }
    rtnOrder.BrokerOrderSeq = static_cast<TThostFtdcSequenceNoType>(OrderSysID);
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(),
        sizeof(rtnOrder.StatusMsg));

    sendRtnOrder();
    return;
}

void CLocalTraderApi::OrderData::handleTrade(const TradePriceVec& tradedPriceInfo, int tradedSize)
{
    if (tradedSize <= 0 || isDone()) return;
    ///今成交数量
    rtnOrder.VolumeTraded += tradedSize;
    rtnOrder.VolumeTotal -= tradedSize;
    rtnOrder.ZCETotalTradedVolume = (strcmp(rtnOrder.ExchangeID, "CZCE") == 0 ? rtnOrder.VolumeTraded : 0);
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    if (rtnOrder.VolumeTraded >= rtnOrder.VolumeTotalOriginal)
    {
        rtnOrder.OrderStatus = THOST_FTDC_OST_AllTraded;
    }
    else
    {
        rtnOrder.OrderStatus = THOST_FTDC_OST_PartTradedQueueing;
    }
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(),
        sizeof(rtnOrder.StatusMsg));


    std::vector<CThostFtdcTradeFieldWrapper> rtnTradeFromOrder;
    getRtnTrade(tradedPriceInfo, tradedSize, rtnTradeFromOrder);

    for (auto& t : rtnTradeFromOrder)
    {
        api.updateByTrade(t);// it will set commission for trade while handling in updateByTrade
        sendRtnTrade(t);
    }
    rtnTrades.insert(rtnTrades.end(), rtnTradeFromOrder.begin(), rtnTradeFromOrder.end());

    sendRtnOrder();
    return;
}

void CLocalTraderApi::OrderData::handleCancel(bool cancelFromClient)
{
    if (isDone()) return;
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    rtnOrder.OrderStatus = THOST_FTDC_OST_Canceled;
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(),
        sizeof(rtnOrder.StatusMsg));

    const CLeeDateTime now_time = CLocalTraderApi::getNowTime(); //CLeeDateTime::GetCurrentTime();
    ///成交时间
    strncpy(rtnOrder.CancelTime, now_time.Format("%H:%M:%S").c_str(),
        sizeof(rtnOrder.CancelTime));
    if (cancelFromClient)
    {
        strncpy(rtnOrder.ActiveUserID, rtnOrder.UserID, sizeof(rtnOrder.ActiveUserID));
    }

    api.updateByCancel(rtnOrder);
    sendRtnOrder();
}

void CLocalTraderApi::OrderData::getRtnTrade(const TradePriceVec& tradePriceVec,
    int tradedSize, std::vector<CThostFtdcTradeFieldWrapper>& Trades)
{
    std::vector<std::string> SingleContracts;
    CLocalTraderApi::GetSingleContractFromCombinationContract(rtnOrder.InstrumentID, SingleContracts);
    if (tradePriceVec.size() != SingleContracts.size())
    {
        return;
    }
    for (size_t _index = 0; _index != SingleContracts.size(); ++_index)
    {
        const auto& instr = SingleContracts[_index];
        CThostFtdcTradeField Trade = { 0 };
        ///经纪公司代码
        strncpy(Trade.BrokerID, rtnOrder.BrokerID, sizeof(Trade.BrokerID));
        ///投资者代码
        strncpy(Trade.InvestorID, rtnOrder.InvestorID, sizeof(Trade.InvestorID));
        ///合约代码
        strncpy(Trade.InstrumentID, instr.c_str(), sizeof(Trade.InstrumentID));
        ///报单引用
        strncpy(Trade.OrderRef, rtnOrder.OrderRef, sizeof(Trade.OrderRef));
        ///用户代码
        strncpy(Trade.UserID, rtnOrder.UserID, sizeof(Trade.UserID));
        ///交易所代码
        strncpy(Trade.ExchangeID, rtnOrder.ExchangeID, sizeof(Trade.ExchangeID));
        ///成交编号
        strncpy(Trade.TradeID, std::to_string(api.getNextTradeID(Trade.ExchangeID)).c_str(), sizeof(Trade.TradeID));
        if (_index % 2 == 0)//若为第一腿
        {
            ///买卖方向
            Trade.Direction = rtnOrder.Direction;
            ///开平标志
            Trade.OffsetFlag = rtnOrder.CombOffsetFlag[0];
        }
        else//若为第二腿
        {
            ///买卖方向
            Trade.Direction = getOppositeDirection(rtnOrder.Direction);
            ///开平标志
            if (rtnOrder.IsSwapOrder == 0)//非互换单. 第二腿的开平方向与第一腿相同.
            {
                Trade.OffsetFlag = rtnOrder.CombOffsetFlag[0];
            }
            else//互换单. 第二腿的开平方向与第一腿相反.
            {
                Trade.OffsetFlag = (rtnOrder.CombOffsetFlag[0] == THOST_FTDC_OF_Open ?
                    THOST_FTDC_OF_Close: THOST_FTDC_OF_Open);
            }
        }
        ///价格
        Trade.Price = tradePriceVec[_index];
        ///报单编号
        strncpy(Trade.OrderSysID, rtnOrder.OrderSysID, sizeof(Trade.OrderSysID));
        ///会员代码
        strncpy(Trade.ParticipantID, rtnOrder.ParticipantID, sizeof(Trade.ParticipantID));
        ///客户代码
        strncpy(Trade.ClientID, rtnOrder.ClientID, sizeof(Trade.ClientID));
        ///交易角色
        Trade.TradingRole = THOST_FTDC_ER_Broker;
        ///合约在交易所的代码
        strncpy(Trade.ExchangeInstID, Trade.InstrumentID, sizeof(Trade.ExchangeInstID));
        ///投机套保标志
        Trade.HedgeFlag = rtnOrder.CombHedgeFlag[0];

        ///数量
        Trade.Volume = tradedSize;
        ///成交时间
        const CLeeDateTime now_time = CLocalTraderApi::getNowTime(); //CLeeDateTime::GetCurrentTime();
        strncpy(Trade.TradeDate, now_time.Format("%Y%m%d").c_str(), sizeof(Trade.TradeDate));
        ///成交时间
        strncpy(Trade.TradeTime, now_time.Format("%H:%M:%S").c_str(), sizeof(Trade.TradeTime));
        ///成交类型(普通成交/组合衍生成交)
        Trade.TradeType = SingleContracts.size() == 1 ?
            THOST_FTDC_TRDT_Common : THOST_FTDC_TRDT_CombinationDerived;
        ///成交价来源
        Trade.PriceSource = THOST_FTDC_PSRC_LastPrice;
        ///交易所交易员代码
        strncpy(Trade.TraderID, rtnOrder.TraderID, sizeof(Trade.TraderID));
        ///本地报单编号
        strncpy(Trade.OrderLocalID, rtnOrder.OrderLocalID, sizeof(Trade.OrderLocalID));
        ///结算会员编号
        strncpy(Trade.ClearingPartID, "", sizeof(Trade.ClearingPartID));
        ///业务单元
        strncpy(Trade.BusinessUnit, rtnOrder.BusinessUnit, sizeof(Trade.BusinessUnit));
        ///序号
        Trade.SequenceNo = rtnOrder.SequenceNo;
        ///交易日
        strncpy(Trade.TradingDay, rtnOrder.TradingDay, sizeof(Trade.TradingDay));
        ///结算编号
        Trade.SettlementID = rtnOrder.SettlementID;
        ///经纪公司报单编号
        Trade.BrokerOrderSeq = rtnOrder.BrokerOrderSeq;
        ///成交来源
        Trade.TradeSource = THOST_FTDC_TSRC_NORMAL;
        ///投资单元代码
        strncpy(Trade.InvestUnitID, rtnOrder.InvestUnitID, sizeof(Trade.InvestUnitID));

        Trades.emplace_back(Trade);
    }
    return;
}

void CLocalTraderApi::OrderData::sendRtnOrder()
{
    api.saveOrderToDb(rtnOrder);
    if (api.getSpi() == nullptr) return;
    api.getSpi()->OnRtnOrder(&rtnOrder);
}

void CLocalTraderApi::OrderData::sendRtnTrade(CThostFtdcTradeFieldWrapper& rtnTrade)
{
    api.saveDataToDb(rtnTrade);
    if (api.getSpi() == nullptr) return;
    api.getSpi()->OnRtnTrade(&(rtnTrade.data));
}

CThostFtdcInvestorPositionDetailField CLocalTraderApi::PositionData::getPositionDetailFromOpenTrade(
    const CThostFtdcTradeField& trade)
{
    CThostFtdcInvestorPositionDetailField posDetail = { 0 };
    if (!isOpen(trade.OffsetFlag)) return posDetail;
    strncpy(posDetail.BrokerID, trade.BrokerID, sizeof(posDetail.BrokerID));
    strncpy(posDetail.InvestorID, trade.InvestorID, sizeof(posDetail.InvestorID));
    strncpy(posDetail.ExchangeID, trade.ExchangeID, sizeof(posDetail.ExchangeID));
    strncpy(posDetail.InstrumentID, trade.InstrumentID, sizeof(posDetail.InstrumentID));
    posDetail.HedgeFlag = trade.HedgeFlag;
    posDetail.OpenPrice = trade.Price;
    strncpy(posDetail.TradingDay, trade.TradingDay, sizeof(posDetail.TradingDay));
    strncpy(posDetail.OpenDate, trade.TradingDay, sizeof(posDetail.OpenDate));
    strncpy(posDetail.TradeID, trade.TradeID, sizeof(posDetail.TradeID));
    posDetail.Volume = trade.Volume;
    posDetail.Direction = trade.Direction;
    posDetail.TradeType = trade.TradeType;
    posDetail.CloseVolume = 0;
    posDetail.LastSettlementPrice = trade.Price;
    //持仓明细中的盈亏等数据并不更新
    return posDetail;
}

// 插入或更新持仓明细
void CLocalTraderApi::PositionData::addPositionDetail(
    const CThostFtdcInvestorPositionDetailField& posDetail)
{
    auto it = std::find_if(posDetailData.begin(), posDetailData.end(),
        [&](const CThostFtdcInvestorPositionDetailField& d) {
            return (strcmp(d.ExchangeID, posDetail.ExchangeID) == 0 &&
                strcmp(d.OpenDate, posDetail.OpenDate) == 0 &&
                strcmp(d.TradeID, posDetail.TradeID) == 0);
        }
    );
    if (it == posDetailData.end())// 若此笔持仓明细还不存在,则插入
    {
        posDetailData.push_back(posDetail);
        sortPositionDetail();// 插入新的持仓明细后,需要对持仓明细排序
    }
    else// 若此笔持仓明细已存在,则更新
        *it = posDetail;
}

#define ACCUMULATE_WITH_DIFFERENT_NAME(FIELD_NAME1, FIELD_NAME2) #FIELD_NAME1 \
" = IFNULL( ( SELECT SUM(" #FIELD_NAME2 ") FROM CThostFtdcInvestorPositionField " \
" WHERE CThostFtdcInvestorPositionField.InvestorID = CThostFtdcTradingAccountField.AccountID AND " \
" CThostFtdcInvestorPositionField.BrokerID = CThostFtdcTradingAccountField.BrokerID), 0) "

#define ACCUMULATE_WITH_SAME_NAME(FIELD) ACCUMULATE_WITH_DIFFERENT_NAME(FIELD, FIELD)

CLocalTraderApi::CSettlementHandler::CSettlementHandler(CSqliteHandler& _sqlHandler)
    : m_sqlHandler(_sqlHandler)
    , m_running(true)
    , m_tradingAccountUpdateFromPositionSql1(
        "UPDATE CThostFtdcTradingAccountField SET "
ACCUMULATE_WITH_SAME_NAME(PositionProfit) ", "
ACCUMULATE_WITH_SAME_NAME(CloseProfit) ", "
ACCUMULATE_WITH_SAME_NAME(Commission) ", "
ACCUMULATE_WITH_SAME_NAME(CashIn) ", "
ACCUMULATE_WITH_DIFFERENT_NAME(CurrMargin,UseMargin) ", "
ACCUMULATE_WITH_SAME_NAME(FrozenMargin) ", "
ACCUMULATE_WITH_SAME_NAME(FrozenCommission) ", "
ACCUMULATE_WITH_SAME_NAME(FrozenCash) ";"
    )
    , m_tradingAccountUpdateFromPositionSql2(
        "UPDATE CThostFtdcTradingAccountField SET "
        " Balance = PreBalance+Deposit-Withdraw+PositionProfit+CloseProfit-Commission+CashIn;"
    )
    , m_tradingAccountUpdateFromPositionSql3(
        "UPDATE CThostFtdcTradingAccountField SET "
        " Available = Balance-CurrMargin-FrozenMargin-FrozenCommission-FrozenCash;"
    )
    , m_sleepSecond(1)
    , m_settlementStartHour(17)
    , m_nextSettlementTime()
    , m_count(0)
    , m_timerThread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));//等待其他线程的initInstrMap初始化好
        CLocalTraderApi::initInstrMap();
        std::string tradingDay = CLocalTraderApi::StaticGetTradingDay();//it can work!
        m_nextSettlementTime.SetDateTime(
            std::stoi(tradingDay.substr(0, 4)),
            std::stoi(tradingDay.substr(4, 2)),
            std::stoi(tradingDay.substr(6, 2)),
            m_settlementStartHour,
            0,
            0);
        std::cout << "[LocalCTP] next Settlement Time is " << m_nextSettlementTime.Format() << std::endl;
        if (checkSettlement())//启动后先判断结算一次
        {
            doSettlement();
        }
        while (m_running)
        {
            //别在一次sleep中sleep太长时间(影响Join的等待时间)
            std::this_thread::sleep_for(std::chrono::seconds(m_sleepSecond));
            if (++m_count >= 60 * 2 / m_sleepSecond)//每隔固定时间间隔,检查是否需要结算
            {
                m_count = 0;
            }
            else
            {
                continue;
            }
            if (checkSettlement())
            {
                doSettlement();
            }
        }
    })
{
}

CLocalTraderApi::CSettlementHandler::~CSettlementHandler()
{
    m_running = false;
    if (m_timerThread.joinable())
        m_timerThread.join();
}

bool CLocalTraderApi::CSettlementHandler::checkSettlement()
{
    // 在什么情况下需要进行结算? 需要满足以下三个条件:
    // 1.当前日期是交易日
    // 2.当前时间大于结算开始时间(如 当前交易日的17:00 )
    // 3.数据库结算表中没有当天的结算记录
    const auto nowTime = CLocalTraderApi::getNowTime(); //CLeeDateTime::GetCurrentTime();
    if (!isTradingDay(nowTime))
    {
        return false;
    }
    if (nowTime < m_nextSettlementTime)
    {
        return false;
    }
    CSqliteHandler::SQL_VALUES sqlValues;
    m_sqlHandler.SelectData(
        "SELECT * FROM 'SettlementData' where TradingDay='" + nowTime.Format("%Y%m%d") + "';",
        sqlValues);
    if (sqlValues.empty())
    {
        return true;
    }
    return false;
}

void CLocalTraderApi::CSettlementHandler::init_format_settlement()
{
    static bool bFirstReadFormat = true;

    if (bFirstReadFormat)
    {
        bFirstReadFormat = false;

        Properties prop;
        std::istringstream istrstr(SETTLEMENT_CONTEXT);
        prop.loadProperties(istrstr, '=', false);

        format_settlement_header1 = prop.getValue("settlement_header1", std::string());
        if (!format_settlement_header1.empty())
        {
            format_settlement_header1 = format_settlement_header1.substr(1);
        }
        format_settlement_header2 = prop.getValue("settlement_header2", std::string());
        if (!format_settlement_header2.empty())
        {
            format_settlement_header2 = format_settlement_header2.substr(1);
        }
        format_settlement_header3 = prop.getValue("settlement_header3", std::string());
        if (!format_settlement_header3.empty())
        {
            format_settlement_header3 = format_settlement_header3.substr(1);
        }
        format_settlement_header4 = prop.getValue("settlement_header4", std::string());
        if (!format_settlement_header4.empty())
        {
            format_settlement_header4 = format_settlement_header4.substr(1);
        }
        format_settlement_header5 = prop.getValue("settlement_header5", std::string());
        if (!format_settlement_header5.empty())
        {
            format_settlement_header5 = format_settlement_header5.substr(1);
        }
        format_settlement_header6 = prop.getValue("settlement_header6", std::string());
        if (!format_settlement_header6.empty())
        {
            format_settlement_header6 = format_settlement_header6.substr(1);
        }
        format_settlement_header7 = prop.getValue("settlement_header7", std::string());
        if (!format_settlement_header7.empty())
        {
            format_settlement_header7 = format_settlement_header7.substr(1);
        }
        format_settlement_header8 = prop.getValue("settlement_header8", std::string());
        if (!format_settlement_header8.empty())
        {
            format_settlement_header8 = format_settlement_header8.substr(1);
        }
        format_settlement_header9 = prop.getValue("settlement_header9", std::string());
        if (!format_settlement_header9.empty())
        {
            format_settlement_header9 = format_settlement_header9.substr(1);
        }
        format_settlement_header10 = prop.getValue("settlement_header10", std::string());
        if (!format_settlement_header10.empty())
        {
            format_settlement_header10 = format_settlement_header10.substr(1);
        }
        format_settlement_account_summary1 = prop.getValue("settlement_account_summary1", std::string());
        if (!format_settlement_account_summary1.empty())
        {
            format_settlement_account_summary1 = format_settlement_account_summary1.substr(1);
        }
        format_settlement_account_summary2 = prop.getValue("settlement_account_summary2", std::string());
        if (!format_settlement_account_summary2.empty())
        {
            format_settlement_account_summary2 = format_settlement_account_summary2.substr(1);
        }
        format_settlement_account_summary3 = prop.getValue("settlement_account_summary3", std::string());
        if (!format_settlement_account_summary3.empty())
        {
            format_settlement_account_summary3 = format_settlement_account_summary3.substr(1);
        }
        format_settlement_account_summary4 = prop.getValue("settlement_account_summary4", std::string());
        if (!format_settlement_account_summary4.empty())
        {
            format_settlement_account_summary4 = format_settlement_account_summary4.substr(1);
        }
        format_settlement_account_summary5 = prop.getValue("settlement_account_summary5", std::string());
        if (!format_settlement_account_summary5.empty())
        {
            format_settlement_account_summary5 = format_settlement_account_summary5.substr(1);
        }
        format_settlement_account_summary6 = prop.getValue("settlement_account_summary6", std::string());
        if (!format_settlement_account_summary6.empty())
        {
            format_settlement_account_summary6 = format_settlement_account_summary6.substr(1);
        }
        format_settlement_account_summary7 = prop.getValue("settlement_account_summary7", std::string());
        if (!format_settlement_account_summary7.empty())
        {
            format_settlement_account_summary7 = format_settlement_account_summary7.substr(1);
        }
        format_settlement_account_summary8 = prop.getValue("settlement_account_summary8", std::string());
        if (!format_settlement_account_summary8.empty())
        {
            format_settlement_account_summary8 = format_settlement_account_summary8.substr(1);
        }
        format_settlement_account_summary9 = prop.getValue("settlement_account_summary9", std::string());
        if (!format_settlement_account_summary9.empty())
        {
            format_settlement_account_summary9 = format_settlement_account_summary9.substr(1);
        }
        format_settlement_account_summary10 = prop.getValue("settlement_account_summary10", std::string());
        if (!format_settlement_account_summary10.empty())
        {
            format_settlement_account_summary10 = format_settlement_account_summary10.substr(1);
        }
        format_settlement_account_summary11 = prop.getValue("settlement_account_summary11", std::string());
        if (!format_settlement_account_summary11.empty())
        {
            format_settlement_account_summary11 = format_settlement_account_summary11.substr(1);
        }
        format_settlement_account_summary12 = prop.getValue("settlement_account_summary12", std::string());
        if (!format_settlement_account_summary12.empty())
        {
            format_settlement_account_summary12 = format_settlement_account_summary12.substr(1);
        }
        format_settlement_account_summary13 = prop.getValue("settlement_account_summary13", std::string());
        if (!format_settlement_account_summary13.empty())
        {
            format_settlement_account_summary13 = format_settlement_account_summary13.substr(1);
        }
        format_settlement_account_summary14 = prop.getValue("settlement_account_summary14", std::string());
        if (!format_settlement_account_summary14.empty())
        {
            format_settlement_account_summary14 = format_settlement_account_summary14.substr(1);
        }
        format_settlement_account_summary15 = prop.getValue("settlement_account_summary15", std::string());
        if (!format_settlement_account_summary15.empty())
        {
            format_settlement_account_summary15 = format_settlement_account_summary15.substr(1);
        }
        format_settlement_account_summary16 = prop.getValue("settlement_account_summary16", std::string());
        if (!format_settlement_account_summary16.empty())
        {
            format_settlement_account_summary16 = format_settlement_account_summary16.substr(1);
        }
        format_settlement_deposit_withdrawal_head1 = prop.getValue("settlement_deposit_withdrawal_head1", std::string());
        if (!format_settlement_deposit_withdrawal_head1.empty())
        {
            format_settlement_deposit_withdrawal_head1 = format_settlement_deposit_withdrawal_head1.substr(1);
        }
        format_settlement_deposit_withdrawal_head2 = prop.getValue("settlement_deposit_withdrawal_head2", std::string());
        if (!format_settlement_deposit_withdrawal_head2.empty())
        {
            format_settlement_deposit_withdrawal_head2 = format_settlement_deposit_withdrawal_head2.substr(1);
        }
        format_settlement_deposit_withdrawal_head3 = prop.getValue("settlement_deposit_withdrawal_head3", std::string());
        if (!format_settlement_deposit_withdrawal_head3.empty())
        {
            format_settlement_deposit_withdrawal_head3 = format_settlement_deposit_withdrawal_head3.substr(1);
        }
        format_settlement_deposit_withdrawal_head4 = prop.getValue("settlement_deposit_withdrawal_head4", std::string());
        if (!format_settlement_deposit_withdrawal_head4.empty())
        {
            format_settlement_deposit_withdrawal_head4 = format_settlement_deposit_withdrawal_head4.substr(1);
        }
        format_settlement_deposit_withdrawal_head5 = prop.getValue("settlement_deposit_withdrawal_head5", std::string());
        if (!format_settlement_deposit_withdrawal_head5.empty())
        {
            format_settlement_deposit_withdrawal_head5 = format_settlement_deposit_withdrawal_head5.substr(1);
        }
        format_settlement_deposit_withdrawal_single_record1 = prop.getValue("settlement_deposit_withdrawal_single_record1", std::string());
        if (!format_settlement_deposit_withdrawal_single_record1.empty())
        {
            format_settlement_deposit_withdrawal_single_record1 = format_settlement_deposit_withdrawal_single_record1.substr(1);
        }
        format_settlement_deposit_withdrawal_end1 = prop.getValue("settlement_deposit_withdrawal_end1", std::string());
        if (!format_settlement_deposit_withdrawal_end1.empty())
        {
            format_settlement_deposit_withdrawal_end1 = format_settlement_deposit_withdrawal_end1.substr(1);
        }
        format_settlement_deposit_withdrawal_end2 = prop.getValue("settlement_deposit_withdrawal_end2", std::string());
        if (!format_settlement_deposit_withdrawal_end2.empty())
        {
            format_settlement_deposit_withdrawal_end2 = format_settlement_deposit_withdrawal_end2.substr(1);
        }
        format_settlement_deposit_withdrawal_end3 = prop.getValue("settlement_deposit_withdrawal_end3", std::string());
        if (!format_settlement_deposit_withdrawal_end3.empty())
        {
            format_settlement_deposit_withdrawal_end3 = format_settlement_deposit_withdrawal_end3.substr(1);
        }
        format_settlement_deposit_withdrawal_end4 = prop.getValue("settlement_deposit_withdrawal_end4", std::string());
        if (!format_settlement_deposit_withdrawal_end4.empty())
        {
            format_settlement_deposit_withdrawal_end4 = format_settlement_deposit_withdrawal_end4.substr(1);
        }
        format_settlement_deposit_withdrawal_end5 = prop.getValue("settlement_deposit_withdrawal_end5", std::string());
        if (!format_settlement_deposit_withdrawal_end5.empty())
        {
            format_settlement_deposit_withdrawal_end5 = format_settlement_deposit_withdrawal_end5.substr(1);
        }
        format_settlement_trade_head1 = prop.getValue("settlement_trade_head1", std::string());
        if (!format_settlement_trade_head1.empty())
        {
            format_settlement_trade_head1 = format_settlement_trade_head1.substr(1);
        }
        format_settlement_trade_head2 = prop.getValue("settlement_trade_head2", std::string());
        if (!format_settlement_trade_head2.empty())
        {
            format_settlement_trade_head2 = format_settlement_trade_head2.substr(1);
        }
        format_settlement_trade_head3 = prop.getValue("settlement_trade_head3", std::string());
        if (!format_settlement_trade_head3.empty())
        {
            format_settlement_trade_head3 = format_settlement_trade_head3.substr(1);
        }
        format_settlement_trade_head4 = prop.getValue("settlement_trade_head4", std::string());
        if (!format_settlement_trade_head4.empty())
        {
            format_settlement_trade_head4 = format_settlement_trade_head4.substr(1);
        }
        format_settlement_trade_head5 = prop.getValue("settlement_trade_head5", std::string());
        if (!format_settlement_trade_head5.empty())
        {
            format_settlement_trade_head5 = format_settlement_trade_head5.substr(1);
        }
        format_settlement_trade_single_record1 = prop.getValue("settlement_trade_single_record1", std::string());
        if (!format_settlement_trade_single_record1.empty())
        {
            format_settlement_trade_single_record1 = format_settlement_trade_single_record1.substr(1);
        }
        format_settlement_trade_end1 = prop.getValue("settlement_trade_end1", std::string());
        if (!format_settlement_trade_end1.empty())
        {
            format_settlement_trade_end1 = format_settlement_trade_end1.substr(1);
        }
        format_settlement_trade_end2 = prop.getValue("settlement_trade_end2", std::string());
        if (!format_settlement_trade_end2.empty())
        {
            format_settlement_trade_end2 = format_settlement_trade_end2.substr(1);
        }
        format_settlement_trade_end3 = prop.getValue("settlement_trade_end3", std::string());
        if (!format_settlement_trade_end3.empty())
        {
            format_settlement_trade_end3 = format_settlement_trade_end3.substr(1);
        }
        format_settlement_trade_end4 = prop.getValue("settlement_trade_end4", std::string());
        if (!format_settlement_trade_end4.empty())
        {
            format_settlement_trade_end4 = format_settlement_trade_end4.substr(1);
        }
        format_settlement_trade_end5 = prop.getValue("settlement_trade_end5", std::string());
        if (!format_settlement_trade_end5.empty())
        {
            format_settlement_trade_end5 = format_settlement_trade_end5.substr(1);
        }
        format_settlement_trade_end6 = prop.getValue("settlement_trade_end6", std::string());
        if (!format_settlement_trade_end6.empty())
        {
            format_settlement_trade_end6 = format_settlement_trade_end6.substr(1);
        }
        format_settlement_trade_end7 = prop.getValue("settlement_trade_end7", std::string());
        if (!format_settlement_trade_end7.empty())
        {
            format_settlement_trade_end7 = format_settlement_trade_end7.substr(1);
        }
        format_settlement_trade_end8 = prop.getValue("settlement_trade_end8", std::string());
        if (!format_settlement_trade_end8.empty())
        {
            format_settlement_trade_end8 = format_settlement_trade_end8.substr(1);
        }
        format_settlement_position_closed_head1 = prop.getValue("settlement_position_closed_head1", std::string());
        if (!format_settlement_position_closed_head1.empty())
        {
            format_settlement_position_closed_head1 = format_settlement_position_closed_head1.substr(1);
        }
        format_settlement_position_closed_head2 = prop.getValue("settlement_position_closed_head2", std::string());
        if (!format_settlement_position_closed_head2.empty())
        {
            format_settlement_position_closed_head2 = format_settlement_position_closed_head2.substr(1);
        }
        format_settlement_position_closed_head3 = prop.getValue("settlement_position_closed_head3", std::string());
        if (!format_settlement_position_closed_head3.empty())
        {
            format_settlement_position_closed_head3 = format_settlement_position_closed_head3.substr(1);
        }
        format_settlement_position_closed_head4 = prop.getValue("settlement_position_closed_head4", std::string());
        if (!format_settlement_position_closed_head4.empty())
        {
            format_settlement_position_closed_head4 = format_settlement_position_closed_head4.substr(1);
        }
        format_settlement_position_closed_head5 = prop.getValue("settlement_position_closed_head5", std::string());
        if (!format_settlement_position_closed_head5.empty())
        {
            format_settlement_position_closed_head5 = format_settlement_position_closed_head5.substr(1);
        }
        format_settlement_position_closed_single_record1 = prop.getValue("settlement_position_closed_single_record1", std::string());
        if (!format_settlement_position_closed_single_record1.empty())
        {
            format_settlement_position_closed_single_record1 = format_settlement_position_closed_single_record1.substr(1);
        }
        format_settlement_position_closed_end1 = prop.getValue("settlement_position_closed_end1", std::string());
        if (!format_settlement_position_closed_end1.empty())
        {
            format_settlement_position_closed_end1 = format_settlement_position_closed_end1.substr(1);
        }
        format_settlement_position_closed_end2 = prop.getValue("settlement_position_closed_end2", std::string());
        if (!format_settlement_position_closed_end2.empty())
        {
            format_settlement_position_closed_end2 = format_settlement_position_closed_end2.substr(1);
        }
        format_settlement_position_closed_end3 = prop.getValue("settlement_position_closed_end3", std::string());
        if (!format_settlement_position_closed_end3.empty())
        {
            format_settlement_position_closed_end3 = format_settlement_position_closed_end3.substr(1);
        }
        format_settlement_position_closed_end4 = prop.getValue("settlement_position_closed_end4", std::string());
        if (!format_settlement_position_closed_end4.empty())
        {
            format_settlement_position_closed_end4 = format_settlement_position_closed_end4.substr(1);
        }
        format_settlement_position_closed_end5 = prop.getValue("settlement_position_closed_end5", std::string());
        if (!format_settlement_position_closed_end5.empty())
        {
            format_settlement_position_closed_end5 = format_settlement_position_closed_end5.substr(1);
        }
        format_settlement_position_closed_end6 = prop.getValue("settlement_position_closed_end6", std::string());
        if (!format_settlement_position_closed_end6.empty())
        {
            format_settlement_position_closed_end6 = format_settlement_position_closed_end6.substr(1);
        }
        format_settlement_position_detail_head1 = prop.getValue("settlement_position_detail_head1", std::string());
        if (!format_settlement_position_detail_head1.empty())
        {
            format_settlement_position_detail_head1 = format_settlement_position_detail_head1.substr(1);
        }
        format_settlement_position_detail_head2 = prop.getValue("settlement_position_detail_head2", std::string());
        if (!format_settlement_position_detail_head2.empty())
        {
            format_settlement_position_detail_head2 = format_settlement_position_detail_head2.substr(1);
        }
        format_settlement_position_detail_head3 = prop.getValue("settlement_position_detail_head3", std::string());
        if (!format_settlement_position_detail_head3.empty())
        {
            format_settlement_position_detail_head3 = format_settlement_position_detail_head3.substr(1);
        }
        format_settlement_position_detail_head4 = prop.getValue("settlement_position_detail_head4", std::string());
        if (!format_settlement_position_detail_head4.empty())
        {
            format_settlement_position_detail_head4 = format_settlement_position_detail_head4.substr(1);
        }
        format_settlement_position_detail_head5 = prop.getValue("settlement_position_detail_head5", std::string());
        if (!format_settlement_position_detail_head5.empty())
        {
            format_settlement_position_detail_head5 = format_settlement_position_detail_head5.substr(1);
        }
        format_settlement_position_detail_single_record1 = prop.getValue("settlement_position_detail_single_record1", std::string());
        if (!format_settlement_position_detail_single_record1.empty())
        {
            format_settlement_position_detail_single_record1 = format_settlement_position_detail_single_record1.substr(1);
        }
        format_settlement_position_detail_end1 = prop.getValue("settlement_position_detail_end1", std::string());
        if (!format_settlement_position_detail_end1.empty())
        {
            format_settlement_position_detail_end1 = format_settlement_position_detail_end1.substr(1);
        }
        format_settlement_position_detail_end2 = prop.getValue("settlement_position_detail_end2", std::string());
        if (!format_settlement_position_detail_end2.empty())
        {
            format_settlement_position_detail_end2 = format_settlement_position_detail_end2.substr(1);
        }
        format_settlement_position_detail_end3 = prop.getValue("settlement_position_detail_end3", std::string());
        if (!format_settlement_position_detail_end3.empty())
        {
            format_settlement_position_detail_end3 = format_settlement_position_detail_end3.substr(1);
        }
        format_settlement_position_detail_end4 = prop.getValue("settlement_position_detail_end4", std::string());
        if (!format_settlement_position_detail_end4.empty())
        {
            format_settlement_position_detail_end4 = format_settlement_position_detail_end4.substr(1);
        }
        format_settlement_position_detail_end5 = prop.getValue("settlement_position_detail_end5", std::string());
        if (!format_settlement_position_detail_end5.empty())
        {
            format_settlement_position_detail_end5 = format_settlement_position_detail_end5.substr(1);
        }
        format_settlement_position_detail_end6 = prop.getValue("settlement_position_detail_end6", std::string());
        if (!format_settlement_position_detail_end6.empty())
        {
            format_settlement_position_detail_end6 = format_settlement_position_detail_end6.substr(1);
        }
        format_settlement_position_detail_end7 = prop.getValue("settlement_position_detail_end7", std::string());
        if (!format_settlement_position_detail_end7.empty())
        {
            format_settlement_position_detail_end7 = format_settlement_position_detail_end7.substr(1);
        }
        format_settlement_position_head1 = prop.getValue("settlement_position_head1", std::string());
        if (!format_settlement_position_head1.empty())
        {
            format_settlement_position_head1 = format_settlement_position_head1.substr(1);
        }
        format_settlement_position_head2 = prop.getValue("settlement_position_head2", std::string());
        if (!format_settlement_position_head2.empty())
        {
            format_settlement_position_head2 = format_settlement_position_head2.substr(1);
        }
        format_settlement_position_head3 = prop.getValue("settlement_position_head3", std::string());
        if (!format_settlement_position_head3.empty())
        {
            format_settlement_position_head3 = format_settlement_position_head3.substr(1);
        }
        format_settlement_position_head4 = prop.getValue("settlement_position_head4", std::string());
        if (!format_settlement_position_head4.empty())
        {
            format_settlement_position_head4 = format_settlement_position_head4.substr(1);
        }
        format_settlement_position_head5 = prop.getValue("settlement_position_head5", std::string());
        if (!format_settlement_position_head5.empty())
        {
            format_settlement_position_head5 = format_settlement_position_head5.substr(1);
        }
        format_settlement_position_single_record1 = prop.getValue("settlement_position_single_record1", std::string());
        if (!format_settlement_position_single_record1.empty())
        {
            format_settlement_position_single_record1 = format_settlement_position_single_record1.substr(1);
        }
        format_settlement_position_end1 = prop.getValue("settlement_position_end1", std::string());
        if (!format_settlement_position_end1.empty())
        {
            format_settlement_position_end1 = format_settlement_position_end1.substr(1);
        }
        format_settlement_position_end2 = prop.getValue("settlement_position_end2", std::string());
        if (!format_settlement_position_end2.empty())
        {
            format_settlement_position_end2 = format_settlement_position_end2.substr(1);
        }
        format_settlement_position_end3 = prop.getValue("settlement_position_end3", std::string());
        if (!format_settlement_position_end3.empty())
        {
            format_settlement_position_end3 = format_settlement_position_end3.substr(1);
        }
        format_settlement_position_end4 = prop.getValue("settlement_position_end4", std::string());
        if (!format_settlement_position_end4.empty())
        {
            format_settlement_position_end4 = format_settlement_position_end4.substr(1);
        }
        format_settlement_position_end5 = prop.getValue("settlement_position_end5", std::string());
        if (!format_settlement_position_end5.empty())
        {
            format_settlement_position_end5 = format_settlement_position_end5.substr(1);
        }
        format_settlement_position_end6 = prop.getValue("settlement_position_end6", std::string());
        if (!format_settlement_position_end6.empty())
        {
            format_settlement_position_end6 = format_settlement_position_end6.substr(1);
        }
        format_settlement_position_end7 = prop.getValue("settlement_position_end7", std::string());
        if (!format_settlement_position_end7.empty())
        {
            format_settlement_position_end7 = format_settlement_position_end7.substr(1);
        }
        format_settlement_position_end8 = prop.getValue("settlement_position_end8", std::string());
        if (!format_settlement_position_end8.empty())
        {
            format_settlement_position_end8 = format_settlement_position_end8.substr(1);
        }

    }
    return;
}

void CLocalTraderApi::CSettlementHandler::accumulateTradingAccountFromPosition()
{
    m_sqlHandler.Update(m_tradingAccountUpdateFromPositionSql1);
    m_sqlHandler.Update(m_tradingAccountUpdateFromPositionSql2);
    m_sqlHandler.Update(m_tradingAccountUpdateFromPositionSql3);
}

void CLocalTraderApi::CSettlementHandler::doSettlement()
{
    init_format_settlement();


    const std::string TradingDay = CLocalTraderApi::StaticGetTradingDay();
    std::cout << "[LocalCTP] doSettlement for TradingDay " << TradingDay << std::endl;
    doWorkInitialSettlement(TradingDay);//结算的前期工作
    CSqliteHandler::SQL_VALUES sqlValues;
    m_sqlHandler.SelectData(CThostFtdcTradingAccountFieldWrapper::SELECT_SQL, sqlValues);
    //对资金账户表中的所有账户,逐个结算(无论其登录与否)
    for (const auto& userSqlValue : sqlValues)
    {
        CThostFtdcTradingAccountFieldWrapper tradingAccountFieldWrapper(userSqlValue);

        doGenerateUserSettlement(tradingAccountFieldWrapper, TradingDay);
    }
    const CLeeDateTime dtTradingDay(
        std::stoi(TradingDay.substr(0, 4)),
        std::stoi(TradingDay.substr(4, 2)),
        std::stoi(TradingDay.substr(6, 2)),
        0,
        0,
        0);
    const std::string newTradingDay = getNextTradingDay(dtTradingDay);
    doWorkAfterSettlement(TradingDay, newTradingDay);//结算后的工作

    m_nextSettlementTime.SetDateTime(
        std::stoi(newTradingDay.substr(0, 4)),
        std::stoi(newTradingDay.substr(4, 2)),
        std::stoi(newTradingDay.substr(6, 2)),
        m_settlementStartHour,
        0,
        0);
    return;
}


void CLocalTraderApi::CSettlementHandler::doWorkInitialSettlement(
    const std::string& oldTradingDay)
{
    CSqliteTransactionHandler transactionHandle(m_sqlHandler);

    //更新所有持仓明细, 对每一笔持仓明细, 以用于结算的价格作为其结算价, 计算保证金和盈亏等, 更新持仓明细表,
    //并且修改合约到期的合约的持仓明细, 将其持仓数量修改为0等.
    CSqliteHandler::SQL_VALUES posDetailSqlValues;
    m_sqlHandler.SelectData(CThostFtdcInvestorPositionDetailFieldWrapper::SELECT_SQL, posDetailSqlValues);
    std::vector<CThostFtdcInvestorPositionDetailFieldWrapper> posDetailVec;
    for (const auto& rowData : posDetailSqlValues)
    {
        CThostFtdcInvestorPositionDetailFieldWrapper posDetailWrapper(rowData);
        auto& posDetail = posDetailWrapper.data;
        auto itInstr = CLocalTraderApi::m_instrData.find(posDetail.InstrumentID);
        if (itInstr == CLocalTraderApi::m_instrData.end())
        {
            continue;
        }
        auto itMdData = CLocalTraderApi::m_mdData.find(itInstr->second.InstrumentID);
        if (itMdData != CLocalTraderApi::m_mdData.end())
        {
            //更新持仓明细中的结算价(因为可能用户没有给这个账号的API投喂行情,例如通过别的账户的API来投喂的)
            posDetail.SettlementPrice = itMdData->second.SettlementPrice;
        }
        if (!isOptions(itInstr->second.ProductClass))
        {
            const double positionPrice(strcmp(posDetail.OpenDate, posDetail.TradingDay) == 0 ?
                posDetail.OpenPrice : posDetail.LastSettlementPrice);
            posDetail.PositionProfitByDate = (posDetail.Direction == THOST_FTDC_D_Buy ? 1.0 : -1.0) *
                (posDetail.SettlementPrice - positionPrice) *
                posDetail.Volume * itInstr->second.VolumeMultiple;//更新持仓盈亏
            posDetail.PositionProfitByTrade = (posDetail.Direction == THOST_FTDC_D_Buy ? 1.0 : -1.0) *
                (posDetail.SettlementPrice - posDetail.OpenPrice) *
                posDetail.Volume * itInstr->second.VolumeMultiple;//更新浮动盈亏
        }
        //如果合约已过期, 则将持仓设为0(即模拟强制平仓操作,无交易手续费,不产生交易记录).
        //感谢Q友763606282"啦啦啦"提醒 强制平仓时需考虑持仓盈亏和期权权利金.
        if (std::string(itInstr->second.ExpireDate) <= oldTradingDay)
        {
            posDetail.Volume = 0;
        }
        posDetail.Margin = posDetail.SettlementPrice * posDetail.Volume *
            itInstr->second.VolumeMultiple * posDetail.MarginRateByMoney +
            posDetail.Volume * posDetail.MarginRateByVolume;//更新保证金

        posDetailVec.emplace_back(posDetail);
    }
    // 删除数据库中的持仓记录
    // 如果不删除, 则下方的 REPLACE INTO 的 SQL 语句貌似不生效
    const std::string deletePositionDetailSql = "DELETE FROM CThostFtdcInvestorPositionDetailField;";
    m_sqlHandler.Delete(deletePositionDetailSql);
    // 将更新后的持仓明细的持仓记录, 全部写入到数据库中
    for (auto& posDetail : posDetailVec)
    {
        m_sqlHandler.Insert(posDetail.generateInsertSql());
    }

    //查询所有账户的持仓, 对每一笔持仓, 以用于结算的价格作为其结算价, 计算保证金和盈亏等, 更新持仓表
    //并且修改合约到期的合约的持仓, 将其持仓数量修改为0等.
    //将持仓日期(PositionDate)为昨仓的持仓合并到今仓的持仓中,然后删除多余的持仓记录.
    //
    //Sqlite的Update中, 使用列时用的是旧值而不是修改后的新值,例如(X表,初始时p为5) update X set p=p+1, q=p; 则执行后p=6,q=5
    //所以使用多个Update语句来分步执行更新
    //20230911更新(为支持将昨仓和今仓记录合并): 算了,完全用SQL语句来更新太麻烦了
    //, 最终选择: 从数据库持仓表读取到内存中的持仓变量,然后修改变量,再写回到数据库中
    CSqliteHandler::SQL_VALUES posSqlValues;
    m_sqlHandler.SelectData(CThostFtdcInvestorPositionFieldWrapper::SELECT_SQL, posSqlValues);
    std::vector<CThostFtdcInvestorPositionFieldWrapper> todayPosVec;
    std::vector<CThostFtdcInvestorPositionFieldWrapper> yeterdayPosVec;
    for (const auto& rowData : posSqlValues)
    {
        CThostFtdcInvestorPositionFieldWrapper posWrapper(rowData);
        auto& pos = posWrapper.data;

        auto itInstr = CLocalTraderApi::m_instrData.find(pos.InstrumentID);
        if (itInstr == CLocalTraderApi::m_instrData.end() || itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            continue;
        }
        auto itMdData = CLocalTraderApi::m_mdData.find(itInstr->second.InstrumentID);
        if (itMdData != CLocalTraderApi::m_mdData.end())
        {
            //更新持仓中的结算价(因为可能用户没有给这个账号的API投喂行情,例如通过别的账户的API来投喂的)
            pos.SettlementPrice = itMdData->second.SettlementPrice;
        }
        if (!isOptions(itInstr->second.ProductClass))
        {
            pos.PositionProfit = (pos.PosiDirection == THOST_FTDC_PD_Long ? 1.0 : -1.0) *
                (pos.SettlementPrice * pos.Position * itInstr->second.VolumeMultiple
                    - pos.PositionCost);//更新持仓盈亏
        }
        if (std::string(itInstr->second.ExpireDate) <= oldTradingDay)
        {
            //模拟强制平仓, 对期权合约的持仓按结算价平仓(无交易手续费,不产生交易记录), 更新权利金收支(CashIn)
            if (isOptions(itInstr->second.ProductClass))
            {
                pos.CashIn += (pos.PosiDirection == THOST_FTDC_PD_Long ? 1.0 : -1.0) *
                    pos.SettlementPrice * pos.Position * itInstr->second.VolumeMultiple;
            }
            pos.Position = 0;
        }
        pos.PositionCost = pos.SettlementPrice * pos.Position *
            itInstr->second.VolumeMultiple;//更新持仓成本
        pos.UseMargin = pos.PositionCost * pos.MarginRateByMoney +
            pos.Position * pos.MarginRateByVolume;//更新持仓占用保证金

        if (pos.PositionDate == THOST_FTDC_PSD_Today)
        {
            todayPosVec.emplace_back(posWrapper);
        }
        else
        {
            yeterdayPosVec.emplace_back(posWrapper);
        }
    }

    // 将昨仓的记录, 合并到今仓的持仓记录中
    for (const auto& yesterdayPos : yeterdayPosVec)
    {
        const auto& pos = yesterdayPos.data;
        const auto relatedTodayPosKey = CLocalTraderApi::generatePositionKey(
               pos.InstrumentID,
               getDirectionFromPositionDirection(pos.PosiDirection),
               THOST_FTDC_PSD_Today);
        auto itTodayPos = std::find_if(todayPosVec.begin(), todayPosVec.end(),
            [&](const CThostFtdcInvestorPositionFieldWrapper& p) {
                return strcmp(p.data.BrokerID, pos.BrokerID) == 0 &&
                    strcmp(p.data.InvestorID, pos.InvestorID) == 0 &&
                    CLocalTraderApi::generatePositionKey(p.data) == relatedTodayPosKey;
            });
        if (itTodayPos == todayPosVec.end())
        {
            todayPosVec.emplace_back(yesterdayPos);
        }
        else
        {
            auto& todayPos = itTodayPos->data;
            ///上日持仓
            ADD_TO_TODAY_VALUE(YdPosition);
            ///持仓量
            ADD_TO_TODAY_VALUE(Position);
            ///多头冻结
            ADD_TO_TODAY_VALUE(LongFrozen);
            ///空头冻结
            ADD_TO_TODAY_VALUE(ShortFrozen);
            ///开仓冻结金额
            ADD_TO_TODAY_VALUE(LongFrozenAmount);
            ///开仓冻结金额
            ADD_TO_TODAY_VALUE(ShortFrozenAmount);
            ///开仓量
            ADD_TO_TODAY_VALUE(OpenVolume);
            ///平仓量
            ADD_TO_TODAY_VALUE(CloseVolume);
            ///开仓金额
            ADD_TO_TODAY_VALUE(OpenAmount);
            ///平仓金额
            ADD_TO_TODAY_VALUE(CloseAmount);
            ///持仓成本
            ADD_TO_TODAY_VALUE(PositionCost);
            ///上次占用的保证金
            ADD_TO_TODAY_VALUE(PreMargin);
            ///占用的保证金
            ADD_TO_TODAY_VALUE(UseMargin);
            ///冻结的保证金
            ADD_TO_TODAY_VALUE(FrozenMargin);
            ///冻结的资金
            ADD_TO_TODAY_VALUE(FrozenCash);
            ///冻结的手续费
            ADD_TO_TODAY_VALUE(FrozenCommission);
            ///资金差额
            ADD_TO_TODAY_VALUE(CashIn);
            ///手续费
            ADD_TO_TODAY_VALUE(Commission);
            ///平仓盈亏
            ADD_TO_TODAY_VALUE(CloseProfit);
            ///持仓盈亏
            ADD_TO_TODAY_VALUE(PositionProfit);
            ///开仓成本
            ADD_TO_TODAY_VALUE(OpenCost);
            ///交易所保证金
            ADD_TO_TODAY_VALUE(ExchangeMargin);
            ///组合成交形成的持仓
            ADD_TO_TODAY_VALUE(CombPosition);
            ///组合多头冻结
            ADD_TO_TODAY_VALUE(CombLongFrozen);
            ///组合空头冻结
            ADD_TO_TODAY_VALUE(CombShortFrozen);
            ///逐日盯市平仓盈亏
            ADD_TO_TODAY_VALUE(CloseProfitByDate);
            ///逐笔对冲平仓盈亏
            ADD_TO_TODAY_VALUE(CloseProfitByTrade);
            ///今日持仓
            ADD_TO_TODAY_VALUE(TodayPosition);
        }
    }

    // 删除数据库中的持仓记录
    const std::string deletePositionSql = "DELETE FROM CThostFtdcInvestorPositionField;";
    m_sqlHandler.Delete(deletePositionSql);
    // 将更新后的今仓的持仓记录, 全部写入到数据库中
    for (auto& todayPos : todayPosVec)
    {
        todayPos.data.PositionDate =
            (isSpecialExchange(todayPos.data.ExchangeID) ?
            THOST_FTDC_PSD_History : THOST_FTDC_PSD_Today);
        m_sqlHandler.Insert(todayPos.generateInsertSql());
    }

    //更新所有账户的资金(根据持仓汇总)
    accumulateTradingAccountFromPosition();
}

void CLocalTraderApi::CSettlementHandler::doWorkAfterSettlement(
    const std::string& oldTradingDay, const std::string& newTradingDay)
{
    CSqliteTransactionHandler transactionHandle(m_sqlHandler);

    //结算后更新所有账户的持仓明细
    //1. 删除持仓数量为0的持仓明细(包含合约到期的合约的持仓明细)
    //2. "交易日"改为新的交易日
    //3. 修改"昨结算价"为与结算价相等
    //4. 持仓盈亏与逐日平仓盈亏与逐笔平仓盈亏改为0
    //5. 平仓量和平仓金额改为0
    const std::string positiondetailUpdateAfterSettlementSql1
        = "DELETE FROM CThostFtdcInvestorPositionDetailField WHERE Volume = 0;";
    const std::string positiondetailUpdateAfterSettlementSql2
        = "UPDATE CThostFtdcInvestorPositionDetailField \
SET TradingDay = '" + newTradingDay + "', LastSettlementPrice = SettlementPrice, \
CloseProfitByDate = 0, CloseProfitByTrade = 0, PositionProfitByDate = 0, \
CloseVolume = 0, CloseAmount = 0;";

    m_sqlHandler.Delete(positiondetailUpdateAfterSettlementSql1);
    m_sqlHandler.Update(positiondetailUpdateAfterSettlementSql2);

    //结算后更新所有账户的持仓
    //1. 删除持仓数量为0的持仓(包含合约到期的合约的持仓)
    //2. "交易日"改为新的交易日
    //3. 修改"上次结算价"为与结算价相等
    //4. "逐日平仓盈亏"与"逐笔平仓盈亏"与"持仓盈亏"和 CashIn(资金差额)改为0
    //5. 冻结持仓与冻结持仓金额与组合冻结持仓改为0
    //6. 冻结资金,冻结保证金,冻结手续费改为0
    //7. "上日持仓"改为与持仓数量相等,"上次占用的保证金"(PreMargin)改为与占用的保证金相等
    //8. "开仓量"与"平仓量"与相应的金额改为0
    //9. "手续费"改为0
    //10. TodayPosition(今日持仓) 改为0
    const std::string positionUpdateSqlAfterSettlementSql1 =
        "DELETE FROM CThostFtdcInvestorPositionField WHERE Position = 0;";

    const std::string positionUpdateSqlAfterSettlementSql2 =
        "UPDATE CThostFtdcInvestorPositionField SET TradingDay = '" + newTradingDay
        + "', PreSettlementPrice = SettlementPrice, PreMargin = UseMargin, \
CloseProfit = 0, PositionProfit = 0, CloseProfitByDate = 0, CloseProfitByTrade = 0, \
LongFrozen = 0, ShortFrozen = 0, LongFrozenAmount = 0, ShortFrozenAmount = 0, \
FrozenMargin = 0, FrozenCash = 0, FrozenCommission = 0, YdPosition = Position, \
OpenVolume = 0, CloseVolume = 0, OpenAmount = 0, CloseAmount = 0, \
Commission = 0, TodayPosition = 0, CombLongFrozen = 0, CombShortFrozen = 0, CashIn = 0;";
    m_sqlHandler.Delete(positionUpdateSqlAfterSettlementSql1);
    m_sqlHandler.Update(positionUpdateSqlAfterSettlementSql2);

    //结算后更新所有账户的资金
    //1. "交易日"改为新的交易日
    //2. 静态权益(PreBalance)改为与Balance相等
    //3. PreMargin改为与CurrMargin相等
    //4. 入金与出金改为0
    //5. 最后根据所有持仓的数据来汇总盈亏计算 Balance 等
    const std::string tradingAccountUpdateSqlAfterSettlement
        = "UPDATE CThostFtdcTradingAccountField SET \
TradingDay = '" + newTradingDay + "', \
PreBalance = Balance, PreMargin = CurrMargin, Deposit = 0, Withdraw = 0;";
    m_sqlHandler.Update(tradingAccountUpdateSqlAfterSettlement);
    accumulateTradingAccountFromPosition();

    //结算后, 更新交易日
    CLocalTraderApi::tradingDay = newTradingDay;

    //结算后, 更新各个账户的数据
    for (auto& api : CLocalTraderApi::trade_api_set)
    {
        api->reloadAccountData();
    }
}

struct MergedPositionData
{
    int LongPositon;
    double LongPositonCost;
    int ShortPositon;
    double ShortPositonCost;
    double PositionProfit;
    double UseMargin;
    double SettlementPrice;
    double PreSettlementPrice;

    MergedPositionData(const CThostFtdcInvestorPositionField& pos = { 0 })
    {
        memset(this, 0, sizeof(*this));
        updateByPosition(pos);
    }
    void updateByPosition(const CThostFtdcInvestorPositionField& pos)
    {
        if (pos.PosiDirection == THOST_FTDC_PD_Long)
        {
            LongPositon += pos.Position;
            LongPositonCost += pos.PositionCost;            
        }
        else
        {
            ShortPositon += pos.Position;
            ShortPositonCost += pos.PositionCost;
        }
        PositionProfit += pos.PositionProfit;
        UseMargin += pos.UseMargin;
        SettlementPrice = pos.SettlementPrice;
        PreSettlementPrice = pos.PreSettlementPrice;
    }
};

void CLocalTraderApi::CSettlementHandler::doGenerateUserSettlement(
    const CThostFtdcTradingAccountFieldWrapper& tradingAccountFieldWrapper,
    const std::string& TradingDay)
{
    const auto& data = tradingAccountFieldWrapper.data;
    const std::string brokerID = data.BrokerID;
    const std::string userID = data.AccountID;

    SettlementDataWrapper settlementDataWrapper;
    SettlementData& settlementData = settlementDataWrapper.data;
    strncpy(settlementData.BrokerID, brokerID.c_str(), sizeof(settlementData.BrokerID));
    strncpy(settlementData.InvestorID, userID.c_str(), sizeof(settlementData.InvestorID));
    ///结算单日期(交易日)
    strncpy(settlementData.TradingDay, TradingDay.c_str(), sizeof(settlementData.TradingDay));

    std::ostringstream ostrstr;
    char ch_single_line[512] = { 0 };

    auto buildSettlementHead = [&]() {
        //结算单头部
        sprintf(ch_single_line, format_settlement_header1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header2.c_str(), TradingDay.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header3.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header4.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header5.c_str(), userID.c_str(), userID.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header6.c_str(), TradingDay.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header7.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header8.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header9.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_header10.c_str());
        ostrstr << ch_single_line << "\r\n";
    };
    auto buildSettlementTradingAccount = [&]() {
        double positiveCashIn(0);
        CSqliteHandler::SQL_VALUES tradeSqlValues;
        const std::string selectTradeSql =
            "SELECT SUM(CashIn) AS positiveCashIn FROM 'CThostFtdcTradeField' where BrokerID='"
            + brokerID + "' and InvestorID='" + userID
            + "' and TradingDay = '" + TradingDay
            + "' and CashIn>0;";
        sqlHandler.SelectData(selectTradeSql, tradeSqlValues);
        for (const auto& rowValue : tradeSqlValues)
        {
            try
            {
                //可能没有找到该符合条件的账户成交记录,则positiveCashIn值为空
                positiveCashIn = std::stod(rowValue.at("positiveCashIn"));
            }
            catch (...)
            {
                positiveCashIn = 0;
            }
            break;
        }
        double negativeCashIn = data.CashIn - positiveCashIn;
        // 资金状况
        sprintf(ch_single_line, format_settlement_account_summary1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary2.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary3.c_str(), data.PreBalance, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary4.c_str(), data.Deposit - data.Withdraw, data.Balance);//出入金(入金减出金)和期末结存
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary5.c_str(), data.CloseProfit, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary6.c_str(), data.PositionProfit, data.Balance);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary7.c_str(), 0.0, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary8.c_str(), data.Commission, data.CurrMargin);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary9.c_str(), 0.0, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary10.c_str(), 0.0, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary11.c_str(), 0.0, 0.0);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary12.c_str(), 0.0, data.Balance);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary13.c_str(), 0.0, data.Available);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary14.c_str(), positiveCashIn, 100 * (LEZ(data.Balance) ? 1.0 : data.CurrMargin) / data.Balance);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary15.c_str(), negativeCashIn, (LTZ(data.Available) ? (0 - data.Available) : 0));
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary16.c_str());
        ostrstr << ch_single_line << "\r\n";

        // 出入金.
        // LocalCTP不支持出入金,因此结算表中无出入金记录
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_head1.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_head2.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_head3.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_head4.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_head5.c_str());
        //ostrstr << ch_single_line << "\r\n";

        //sprintf(ch_single_line, "", TradingDay.c_str(), 0.0, 0.0, utf8_to_gbk("入金").c_str());//utf8_to_gbk("出金")
        //ostrstr << ch_single_line << "\r\n";

        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_end1.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_end2.c_str(), 出入金记录条数, 总入金, 总出金);
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_end3.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_end4.c_str());
        //ostrstr << ch_single_line << "\r\n";
        //sprintf(ch_single_line, format_settlement_deposit_withdrawal_end5.c_str());
        //ostrstr << ch_single_line << "\r\n";
    };
    auto buildSettlementTrade = [&]() {
        CSqliteHandler::SQL_VALUES tradeSqlValues;
        const std::string selectTradeSql =
            "SELECT * FROM 'CThostFtdcTradeField' where BrokerID='"
            + brokerID + "' and InvestorID='" + userID
            + "' and TradingDay = '" + TradingDay
            +"' ORDER BY TradeID ASC ;";
        sqlHandler.SelectData(selectTradeSql, tradeSqlValues);
        if (tradeSqlValues.empty())//若没有找到该账户成交记录
        {
            return;
        }
        sprintf(ch_single_line, format_settlement_trade_head1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_head2.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_head3.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_head4.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_head5.c_str());
        ostrstr << ch_single_line << "\r\n";

        int total_Lots = 0;//总成交手数
        double total_Turnover = 0.0;//总成交额
        double total_Fee = 0.0;//总手续费
        double total_CloseProfit = 0.0;//总平仓盈亏
        double total_CashIn = 0.0;//总权利金收支
        for (const auto& tradeValue : tradeSqlValues)
        {
            CThostFtdcTradeFieldWrapper tradeWrapper(tradeValue);
            const auto& trade = tradeWrapper.data;
            auto itInstr = CLocalTraderApi::m_instrData.find(trade.InstrumentID);
            if (itInstr == CLocalTraderApi::m_instrData.end())
            {
                continue;
            }
            const CThostFtdcInstrumentField& ContractInfo = itInstr->second;
            auto itProduct = CLocalTraderApi::m_products.find(ContractInfo.ProductID);
            if (itProduct == CLocalTraderApi::m_products.end())
            {
                continue;
            }

            int thisLots = trade.Volume;//成交手数
            total_Lots += thisLots;
            double thisTurnover = trade.Price * thisLots * ContractInfo.VolumeMultiple;//成交额
            total_Turnover += thisTurnover;
            total_CloseProfit += tradeWrapper.CloseProfit;//平仓盈亏
            total_Fee += tradeWrapper.Commission;//手续费
            total_CashIn += tradeWrapper.CashIn;//权利金收支
            sprintf(ch_single_line, format_settlement_trade_single_record1.c_str(),
                TradingDay.c_str(),
                get_exchange_name(ContractInfo.ExchangeID).c_str(),
                itProduct->second.ProductName,
                trade.InstrumentID,
                get_direction_name(trade.Direction).c_str(),
                trade.Price,
                thisLots,
                thisTurnover,
                get_open_close_name(trade.OffsetFlag).c_str(),
                tradeWrapper.Commission,
                tradeWrapper.CloseProfit,
                tradeWrapper.CashIn,
                trade.TradeID);
            ostrstr << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_trade_end1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end2.c_str(), (int)tradeSqlValues.size(), total_Lots, total_Turnover, total_Fee, total_CloseProfit, total_CashIn);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end3.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end4.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end5.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end6.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end7.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end8.c_str());
        ostrstr << ch_single_line << "\r\n";
    };

    auto buildSettlementCloseDetail = [&]() {
        CSqliteHandler::SQL_VALUES closeDetailSqlValues;
        const std::string selectCloseDetailSql =
            "SELECT * FROM 'CloseDetail' where BrokerID='"
            + brokerID + "' and InvestorID='" + userID
            + "' and CloseDate = '" + TradingDay
            + "';";
        sqlHandler.SelectData(selectCloseDetailSql, closeDetailSqlValues);
        if (closeDetailSqlValues.empty())//若没有找到该账户平仓明细记录
        {
            return;
        }
        sprintf(ch_single_line, format_settlement_position_closed_head1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_head2.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_head3.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_head4.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_head5.c_str());
        ostrstr << ch_single_line << "\r\n";

        int total_position_closed_lots = 0;//总手数
        double total_position_closed_close_profit = 0.0;//总平仓盈亏
        double total_position_closed_cashIn = 0.0;//总权利金收支
        for (const auto& closeDetailValue : closeDetailSqlValues)
        {
            CloseDetailWrapper closeDetailWrapper(closeDetailValue);
            const auto& closeDetail = closeDetailWrapper.data;
            auto itInstr = CLocalTraderApi::m_instrData.find(closeDetail.InstrumentID);
            if (itInstr == CLocalTraderApi::m_instrData.end())
            {
                continue;
            }
            const CThostFtdcInstrumentField& ContractInfo = itInstr->second;
            auto itProduct = CLocalTraderApi::m_products.find(ContractInfo.ProductID);
            if (itProduct == CLocalTraderApi::m_products.end())
            {
                continue;
            }

            total_position_closed_lots += closeDetail.CloseVolume;
            total_position_closed_close_profit += closeDetail.CloseProfit;
            total_position_closed_cashIn += closeDetail.CashIn;

            sprintf(ch_single_line, format_settlement_position_closed_single_record1.c_str(),
                closeDetail.CloseDate,
                get_exchange_name(ContractInfo.ExchangeID).c_str(),
                itProduct->second.ProductName,
                closeDetail.InstrumentID,
                closeDetail.OpenDate,
                get_direction_name(closeDetail.Direction).c_str(),
                closeDetail.CloseVolume,
                closeDetail.OpenPrice,
                closeDetail.PreSettlementPrice,
                closeDetail.ClosePrice,
                closeDetail.CloseProfit,
                closeDetail.CashIn);
            ostrstr << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_position_closed_end1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end2.c_str(), (int)closeDetailSqlValues.size(), total_position_closed_lots, total_position_closed_close_profit, total_position_closed_cashIn);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end3.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end4.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end5.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end6.c_str());
        ostrstr << ch_single_line << "\r\n";
    };

    auto buildSettlementPositionDetail = [&]() {
        CSqliteHandler::SQL_VALUES posDetailSqlValues;
        //从数据库查找持仓数量大于0的持仓明细记录
        const std::string selectPosDetailSql =
            "SELECT * FROM 'CThostFtdcInvestorPositionDetailField' where BrokerID='"
            + brokerID + "' and InvestorID='"
            + userID + "' and Volume != 0;";
            // = CThostFtdcInvestorPositionDetailFieldWrapper::generateSelectSqlByUserID(brokerID, userID);
        sqlHandler.SelectData(selectPosDetailSql, posDetailSqlValues);
        if (posDetailSqlValues.empty())//若没有找到该账户持仓明细记录
        {
            return;
        }

        std::ostringstream ostrstr_position_detail;

        sprintf(ch_single_line, format_settlement_position_detail_head1.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_head2.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_head3.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_head4.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_head5.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";

        int total_Position = 0;//总持仓量
        double total_FloatProfit = 0.0;//总浮动盈亏
        double total_PositionProfit = 0.0;//总持仓盈亏
        double total_Margin = 0.0;//总保证金
        for (const auto& posDetailValue : posDetailSqlValues)
        {
            CThostFtdcInvestorPositionDetailFieldWrapper posDetailWrapper(posDetailValue);
            const auto& posDetail = posDetailWrapper.data;
            auto itInstr = CLocalTraderApi::m_instrData.find(posDetail.InstrumentID);
            if (itInstr == CLocalTraderApi::m_instrData.end())
            {
                continue;
            }
            const CThostFtdcInstrumentField& ContractInfo = itInstr->second;
            auto itProduct = CLocalTraderApi::m_products.find(ContractInfo.ProductID);
            if (itProduct == CLocalTraderApi::m_products.end())
            {
                continue;
            }
            total_Position += posDetail.Volume;//持仓量
            total_FloatProfit += posDetail.PositionProfitByTrade;//浮动盈亏
            total_PositionProfit += posDetail.PositionProfitByDate;//持仓盈亏
            total_Margin += posDetail.Margin;//保证金

            sprintf(ch_single_line, format_settlement_position_detail_single_record1.c_str(),
                get_exchange_name(ContractInfo.ExchangeID).c_str(),
                itProduct->second.ProductName,
                posDetail.InstrumentID,
                posDetail.OpenDate,
                get_direction_name(posDetail.Direction).c_str(),
                posDetail.Volume,
                posDetail.OpenPrice,
                posDetail.LastSettlementPrice,
                posDetail.SettlementPrice,
                posDetail.PositionProfitByTrade,
                posDetail.PositionProfitByDate,
                posDetail.Margin);
            ostrstr_position_detail << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_position_detail_end1.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end2.c_str(), (int)posDetailSqlValues.size(), total_Position, total_FloatProfit, total_PositionProfit, total_Margin);
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end3.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end4.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end5.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end6.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_detail_end7.c_str());
        ostrstr_position_detail << ch_single_line << "\r\n";

        ostrstr << ostrstr_position_detail.str();
    };

    auto buildSettlementPosition = [&]() {
        CSqliteHandler::SQL_VALUES posSqlValues;
        //从数据库查找持仓数量大于0的持仓记录
        const std::string selectPosSql =
            "SELECT * FROM 'CThostFtdcInvestorPositionField' where BrokerID='"
            + brokerID + "' and InvestorID='"
            + userID + "' and Position != 0;";
            // = CThostFtdcInvestorPositionFieldWrapper::generateSelectSqlByUserID(brokerID, userID);
        sqlHandler.SelectData(selectPosSql, posSqlValues);
        if (posSqlValues.empty())//若没有找到该账户持仓记录
        {
            return;
        }

        std::ostringstream ostrstr_position;

        sprintf(ch_single_line, format_settlement_position_head1.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_head2.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_head3.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_head4.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_head5.c_str());
        ostrstr_position << ch_single_line << "\r\n";

        int total_LongPositon = 0;//总买入方向持仓量
        int total_ShortPositon = 0;//总卖出方向持仓量
        double total_PositionProfit = 0.0;//总持仓盈亏
        double total_Margin = 0.0;//总保证金

        //将同一合约的持仓合并的数据(包括多头和空头,今仓和昨仓,全部合在一起)
        std::map<std::string, MergedPositionData> mergedPositions;
        for (const auto& posValue : posSqlValues)
        {
            CThostFtdcInvestorPositionFieldWrapper posWrapper(posValue);
            const auto& pos = posWrapper.data;

            auto itPosInMergedPositions = mergedPositions.find(pos.InstrumentID);
            if (itPosInMergedPositions == mergedPositions.end())//若没找到,则向其中插入该持仓
            {
                MergedPositionData temp(pos);
                mergedPositions.emplace(pos.InstrumentID, temp);
            }
            else
            {
                itPosInMergedPositions->second.updateByPosition(pos);
            }
        }
        for (const auto& posInMergedPositionPair : mergedPositions)
        {
            const auto& posInMergedPosition = posInMergedPositionPair.second;
            auto itInstr = CLocalTraderApi::m_instrData.find(posInMergedPositionPair.first);
            if (itInstr == CLocalTraderApi::m_instrData.end())
            {
                continue;
            }
            const CThostFtdcInstrumentField& ContractInfo = itInstr->second;
            auto itProduct = CLocalTraderApi::m_products.find(ContractInfo.ProductID);
            if (itProduct == CLocalTraderApi::m_products.end())
            {
                continue;
            }

            total_LongPositon += posInMergedPosition.LongPositon;//买入方向持仓量
            total_ShortPositon += posInMergedPosition.ShortPositon;//卖出方向持仓量
            total_PositionProfit += posInMergedPosition.PositionProfit;//持仓盈亏
            total_Margin += posInMergedPosition.UseMargin;//保证金

            //买均价
            const double LongAvgPrice = (posInMergedPosition.LongPositon == 0 ?
                0.0 :
                (posInMergedPosition.LongPositonCost / (1.0 * posInMergedPosition.LongPositon * ContractInfo.VolumeMultiple)));
            //卖均价
            const double ShortAvgPrice = (posInMergedPosition.ShortPositon == 0 ?
                0.0 :
                (posInMergedPosition.ShortPositonCost / (1.0 * posInMergedPosition.ShortPositon * ContractInfo.VolumeMultiple)));
            sprintf(ch_single_line, format_settlement_position_single_record1.c_str(),
                itProduct->second.ProductName,
                posInMergedPositionPair.first.c_str(),
                posInMergedPosition.LongPositon,
                LongAvgPrice,
                posInMergedPosition.ShortPositon,
                ShortAvgPrice,
                posInMergedPosition.PreSettlementPrice,
                posInMergedPosition.SettlementPrice,
                posInMergedPosition.PositionProfit,
                posInMergedPosition.UseMargin);
            ostrstr_position << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_position_end1.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end2.c_str(), (int)mergedPositions.size(), total_LongPositon, total_ShortPositon, total_PositionProfit, total_Margin);
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end3.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end4.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end5.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end6.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end7.c_str());
        ostrstr_position << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_end8.c_str());
        ostrstr_position << ch_single_line << "\r\n";

        ostrstr << ostrstr_position.str();
    };

    buildSettlementHead();//结算单头部
    buildSettlementTradingAccount();//资金状况和出入金
    buildSettlementTrade();//成交记录
    buildSettlementCloseDetail();//平仓明细记录
    buildSettlementPositionDetail();//持仓明细记录
    buildSettlementPosition();//持仓记录

    const std::string str_to_encode = ostrstr.str();
    ///结算单内容
    settlementData.SettlementContent =
        base64_encode(reinterpret_cast<const unsigned char*>(str_to_encode.c_str()),
            static_cast<unsigned int>(str_to_encode.size()));//base64加密结算单正文;

    m_sqlHandler.Insert(settlementDataWrapper.generateInsertSql());
}

} // end namespace localCTP
