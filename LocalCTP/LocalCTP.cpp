#include "stdafx.h"
#include "LocalTraderApi.h"

#define READ_CHAR_ARRAY_MEMBER(m) \
    { std::getline(i, temp, ','); \
      strcpy(instr.m, temp.c_str()); }

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


bool CLocalTraderApi::OrderData::isDone() const
{
    return rtnOrder.OrderStatus != THOST_FTDC_OST_PartTradedQueueing &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_NoTradeQueueing &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_Unknown &&
        rtnOrder.OrderStatus != THOST_FTDC_OST_NotTouched;
}

void CLocalTraderApi::OrderData::DealTestReqOrderInsert_Normal(const CThostFtdcInputOrderField& InputOrder)
{
    // "未知"状态

    ///经纪公司代码
    strcpy(rtnOrder.BrokerID, InputOrder.BrokerID);
    ///投资者代码
    strcpy(rtnOrder.InvestorID, InputOrder.InvestorID);
    ///合约代码
    strcpy(rtnOrder.InstrumentID, InputOrder.InstrumentID);
    ///报单引用
    strcpy(rtnOrder.OrderRef, InputOrder.OrderRef);
    ///用户代码
    strcpy(rtnOrder.UserID, InputOrder.UserID);
    ///报单价格条件
    rtnOrder.OrderPriceType = InputOrder.OrderPriceType;
    ///买卖方向
    rtnOrder.Direction = InputOrder.Direction;
    ///组合开平标志
    strcpy(rtnOrder.CombOffsetFlag, InputOrder.CombOffsetFlag);
    ///组合投机套保标志
    strcpy(rtnOrder.CombHedgeFlag, InputOrder.CombHedgeFlag);
    ///价格
    rtnOrder.LimitPrice = InputOrder.LimitPrice;
    ///数量
    rtnOrder.VolumeTotalOriginal = InputOrder.VolumeTotalOriginal;
    ///有效期类型
    rtnOrder.TimeCondition = InputOrder.TimeCondition;
    ///GTD日期
    strcpy(rtnOrder.GTDDate, "");
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
    strcpy(rtnOrder.BusinessUnit, "");
    ///请求编号
    rtnOrder.RequestID = InputOrder.RequestID;
    ///本地报单编号
    //也使用报单引用
    strcpy(rtnOrder.OrderLocalID, InputOrder.OrderRef);
    ///交易所代码
    strcpy(rtnOrder.ExchangeID, InputOrder.ExchangeID);
    ///会员代码
    strcpy(rtnOrder.ParticipantID, "");
    ///客户代码
    strcpy(rtnOrder.ClientID, "");
    ///合约在交易所的代码
    strcpy(rtnOrder.ExchangeInstID, InputOrder.InstrumentID);
    ///交易所交易员代码
    strcpy(rtnOrder.TraderID, "");
    ///安装编号
    rtnOrder.InstallID = 0;
    ///报单提交状态
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    ///报单提示序号
    rtnOrder.NotifySequence = 0;
    ///交易日
    strcpy(rtnOrder.TradingDay, api.GetTradingDay());
    ///结算编号
    rtnOrder.SettlementID = 0;
    ///报单编号
    strcpy(rtnOrder.OrderSysID, "");
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
    strcpy(rtnOrder.InsertDate, now_time.Format("%Y%m%d").c_str());
    ///委托时间
    strcpy(rtnOrder.InsertTime, now_time.Format("%H:%M:%S").c_str());
    ///激活时间
    strcpy(rtnOrder.ActiveTime, "");
    ///挂起时间
    strcpy(rtnOrder.SuspendTime, "");
    ///最后修改时间
    strcpy(rtnOrder.UpdateTime, "");
    ///撤销时间
    strcpy(rtnOrder.CancelTime, "");
    ///最后修改交易所交易员代码
    strcpy(rtnOrder.ActiveTraderID, "");
    ///结算会员编号
    strcpy(rtnOrder.ClearingPartID, "");
    ///序号
    rtnOrder.SequenceNo = 0;
    ///前置编号
    rtnOrder.FrontID = 0; // is 0, all the same
    ///会话编号
    rtnOrder.SessionID = 0; // is 0, all the same
    ///用户端产品信息
    strcpy(rtnOrder.UserProductInfo, "");
    ///状态信息
    strcpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str());
    ///用户强评标志
    rtnOrder.UserForceClose = InputOrder.UserForceClose;
    ///操作用户代码
    strcpy(rtnOrder.ActiveUserID, "");
    ///经纪公司报单编号
    rtnOrder.BrokerOrderSeq = 0;
    ///相关报单
    strcpy(rtnOrder.RelativeOrderSysID, "");
    ///郑商所成交数量
    rtnOrder.ZCETotalTradedVolume = (strcmp(InputOrder.ExchangeID, "CZCE") == 0 ? rtnOrder.VolumeTraded : 0);
    ///互换单标志
    rtnOrder.IsSwapOrder = InputOrder.IsSwapOrder;
    ///营业部编号
    strcpy(rtnOrder.BranchID, "");
    ///投资单元代码
    strcpy(rtnOrder.InvestUnitID, "");
    ///资金账号
    strcpy(rtnOrder.AccountID, InputOrder.AccountID);
    ///币种代码
    strcpy(rtnOrder.CurrencyID, "CNY");

    api.getSpi()->OnRtnOrder(&rtnOrder);

    // "未知"-> "未成交" 状态 
    int OrderSysID = ++api.getOrderSysID();
    strcpy(rtnOrder.OrderSysID, std::to_string(OrderSysID).c_str());
    rtnOrder.BrokerOrderSeq = OrderSysID;
    rtnOrder.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    rtnOrder.OrderStatus = THOST_FTDC_OST_NoTradeQueueing;
    strcpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str());

    api.getSpi()->OnRtnOrder(&rtnOrder);
    return;
}

void CLocalTraderApi::OrderData::handleTrade(double tradedPrice, int tradedSize)
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
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(), sizeof(rtnOrder.StatusMsg));


    std::vector<CThostFtdcTradeField> rtnTradeFromOrder;
    getRtnTrade(tradedPrice, tradedSize, rtnTradeFromOrder);
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
    strncpy(rtnOrder.StatusMsg, getStatusMsgByStatus(rtnOrder.OrderStatus).c_str(), sizeof(rtnOrder.StatusMsg));

    const CLeeDateTime now_time = CLeeDateTime::GetCurrentTime();
    ///成交时间
    strcpy(rtnOrder.CancelTime, now_time.Format("%H:%M:%S").c_str());
    if (cancelFromClient)
    {
        strcpy(rtnOrder.ActiveUserID, rtnOrder.UserID);
    }

    api.updateByCancel(rtnOrder);
    sendRtnOrder();
}

void CLocalTraderApi::OrderData::getRtnTrade(double trade_price, int tradedSize, std::vector<CThostFtdcTradeField>& Trades)
{
    std::vector<std::string> SingleContracts;
    CLocalTraderApi::GetSingleContractFromCombinationContract(rtnOrder.InstrumentID, SingleContracts);
    for (size_t _index = 0; _index != SingleContracts.size(); ++_index)
    {
        CThostFtdcTradeField Trade = { 0 };
        ///经纪公司代码
        strcpy(Trade.BrokerID, rtnOrder.BrokerID);
        ///投资者代码
        strcpy(Trade.InvestorID, rtnOrder.InvestorID);
        ///合约代码
        strcpy(Trade.InstrumentID, SingleContracts[_index].c_str());
        ///报单引用
        strcpy(Trade.OrderRef, rtnOrder.OrderRef);
        ///用户代码
        strcpy(Trade.UserID, rtnOrder.UserID);
        ///交易所代码
        strcpy(Trade.ExchangeID, rtnOrder.ExchangeID);
        ///成交编号
        strcpy(Trade.TradeID, std::to_string(++api.getTradeID()).c_str());
        if (_index % 2 == 0)//若为第一腿
        {
            ///买卖方向
            Trade.Direction = rtnOrder.Direction;
            ///价格
            Trade.Price = trade_price;
        }
        else//若为第二腿
        {
            ///买卖方向
            Trade.Direction =
                (rtnOrder.Direction == THOST_FTDC_D_Buy ? THOST_FTDC_D_Sell : THOST_FTDC_D_Buy);
            ///价格
            Trade.Price = 0.0;
        }
        ///报单编号
        strcpy(Trade.OrderSysID, rtnOrder.OrderSysID);
        ///会员代码
        strcpy(Trade.ParticipantID, rtnOrder.ParticipantID);
        ///客户代码
        strcpy(Trade.ClientID, rtnOrder.ClientID);
        ///交易角色
        Trade.TradingRole = THOST_FTDC_ER_Broker;
        ///合约在交易所的代码
        strcpy(Trade.ExchangeInstID, Trade.InstrumentID);
        ///开平标志
        Trade.OffsetFlag = rtnOrder.CombOffsetFlag[0];
        ///投机套保标志
        Trade.HedgeFlag = rtnOrder.CombHedgeFlag[0];

        ///数量
        Trade.Volume = tradedSize;
        ///成交时期
        const CLeeDateTime now_time = CLeeDateTime::GetCurrentTime();
        strcpy(Trade.TradeDate, now_time.Format("%Y%m%d").c_str());
        ///成交时间
        strcpy(Trade.TradeTime, now_time.Format("%H:%M:%S").c_str());
        ///成交类型(普通成交/组合衍生成交)
        Trade.TradeType = SingleContracts.size() == 1 ?
            THOST_FTDC_TRDT_Common : THOST_FTDC_TRDT_CombinationDerived;
        ///成交价来源
        Trade.PriceSource = THOST_FTDC_PSRC_LastPrice;
        ///交易所交易员代码
        strcpy(Trade.TraderID, rtnOrder.TraderID);
        ///本地报单编号
        strcpy(Trade.OrderLocalID, rtnOrder.OrderLocalID);
        ///结算会员编号
        strcpy(Trade.ClearingPartID, "");
        ///业务单元
        strcpy(Trade.BusinessUnit, rtnOrder.BusinessUnit);
        ///序号
        Trade.SequenceNo = rtnOrder.SequenceNo;
        ///交易日
        strcpy(Trade.TradingDay, rtnOrder.TradingDay);
        ///结算编号
        Trade.SettlementID = rtnOrder.SettlementID;
        ///经纪公司报单编号
        Trade.BrokerOrderSeq = rtnOrder.BrokerOrderSeq;
        ///成交来源
        Trade.TradeSource = THOST_FTDC_TSRC_NORMAL;
        ///投资单元代码
        strcpy(Trade.InvestUnitID, rtnOrder.InvestUnitID);

        Trades.emplace_back(Trade);
    }
    return;
}

void CLocalTraderApi::OrderData::sendRtnOrder()
{
    api.getSpi()->OnRtnOrder(&rtnOrder);
}

void CLocalTraderApi::OrderData::sendRtnTrade(CThostFtdcTradeField& rtnTrade)
{
    api.getSpi()->OnRtnTrade(&rtnTrade);
}
