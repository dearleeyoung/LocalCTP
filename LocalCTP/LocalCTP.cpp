#include "stdafx.h"
#include "LocalTraderApi.h"

#define READ_CHAR_ARRAY_MEMBER(m) \
    { std::getline(i, temp, ','); \
      strncpy(instr.m, temp.c_str(), sizeof(instr.m)); }

#define READ_MEMBER(m) \
    { std::getline(i, temp, ','); \
      std::istringstream iss(temp); \
      iss >> instr.m; }

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
    const CLeeDateTime now_time = CLeeDateTime::GetCurrentTime();
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


    std::vector<CThostFtdcTradeField> rtnTradeFromOrder;
    getRtnTrade(tradedPriceInfo, tradedSize, rtnTradeFromOrder);
    rtnTrades.insert(rtnTrades.end(), rtnTradeFromOrder.begin(), rtnTradeFromOrder.end());

    for (auto& t : rtnTradeFromOrder)
    {
        api.updateByTrade(t);
        sendRtnTrade(t);
    }
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

    const CLeeDateTime now_time = CLeeDateTime::GetCurrentTime();
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
    int tradedSize, std::vector<CThostFtdcTradeField>& Trades)
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
        }
        else//若为第二腿
        {
            ///买卖方向
            Trade.Direction = getOppositeDirection(rtnOrder.Direction);
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
        ///开平标志
        Trade.OffsetFlag = rtnOrder.CombOffsetFlag[0];
        ///投机套保标志
        Trade.HedgeFlag = rtnOrder.CombHedgeFlag[0];

        ///数量
        Trade.Volume = tradedSize;
        ///成交时期
        const CLeeDateTime now_time = CLeeDateTime::GetCurrentTime();
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

void CLocalTraderApi::OrderData::sendRtnTrade(CThostFtdcTradeField& rtnTrade)
{
    api.saveTradeToDb(rtnTrade);
    if (api.getSpi() == nullptr) return;
    api.getSpi()->OnRtnTrade(&rtnTrade);
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
    strncpy(posDetail.TradeID, trade.TradeID, sizeof(posDetail.TradeID));
    posDetail.Volume = trade.Volume;
    posDetail.Direction = trade.Direction;
    posDetail.TradeType = trade.TradeType;
    posDetail.CloseVolume = 0;
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
