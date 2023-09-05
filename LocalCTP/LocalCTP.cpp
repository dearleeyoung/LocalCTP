#include "stdafx.h"
#include "LocalTraderApi.h"
#include "Properties.h"

#define READ_CHAR_ARRAY_MEMBER(m) \
    { std::getline(i, temp, ','); \
      strncpy(instr.m, temp.c_str(), sizeof(instr.m)); }

#define READ_MEMBER(m) \
    { std::getline(i, temp, ','); \
      std::istringstream iss(temp); \
      iss >> instr.m; }

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
    posDetail.SettlementPrice = trade.Price;
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

CLocalTraderApi::CSettlementHandler::CSettlementHandler(CSqliteHandler& _sqlHandler)
    : m_sqlHandler(_sqlHandler)
    , m_running(true)
    , m_sleepSecond(1)
    , m_settlementStartHour(16)
    , m_count(0)
    , m_timerThread([this]() {
        CLocalTraderApi::initInstrMap();
        if (!checkSettlement())//启动后先判断结算一次
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
            if (!checkSettlement())
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
    // 2.当前时间大于结算开始时间(如 16:00 )
    // 3.数据库结算表中没有当天的结算记录
    const auto nowTime = CLeeDateTime::GetCurrentTime();
    if (!isTradingDay(nowTime))
    {
        return true;
    }
    if (nowTime.GetHour() < m_settlementStartHour)
    {
        return true;
    }
    CSqliteHandler::SQL_VALUES sqlValues;
    m_sqlHandler.SelectData(
        "SELECT * FROM 'SettlementData' where TradingDay='" + nowTime.Format("%Y%m%d") + "';",
        sqlValues);
    if (sqlValues.empty())
    {
        return false;
    }
    return true;
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

void CLocalTraderApi::CSettlementHandler::doSettlement()
{
    init_format_settlement();

    CSqliteHandler::SQL_VALUES sqlValues;
    m_sqlHandler.SelectData(CThostFtdcTradingAccountFieldWrapper::SELECT_SQL, sqlValues);
    const CLeeDateTime nowTime = CLeeDateTime::GetCurrentTime();
    const std::string TradingDay = nowTime.Format("%Y%m%d");
    for (const auto& userSqlValue : sqlValues)
    {
        CThostFtdcTradingAccountFieldWrapper tradingAccountFieldWrapper(userSqlValue);

        doUserSettlement(tradingAccountFieldWrapper, TradingDay);
    }
    const std::string newTradingDay = getNextTradingDay(TradingDay);
    doWorkAfterSettlement(TradingDay, newTradingDay);//结算后的工作
    return;
}

void CLocalTraderApi::CSettlementHandler::doWorkAfterSettlement(
    const std::string& oldTradingDay, const std::string& newTradingDay)
{
    //结算后更新所有账户的持仓明细
    //1. 删除持仓数量为0的持仓明细
    //2. "交易日"改为新的交易日
    //3. 修改"昨结算价"为与结算价相等
    //4. 持仓盈亏与逐日平仓盈亏与逐笔平仓盈亏改为0
    //5. 平仓量和平仓金额改为0
    //6. 删除合约到期的合约的持仓明细.
    const std::string positiondetailUpdateAfterSettlementSql1
        = "DELETE FROM CThostFtdcInvestorPositionDetailField WHERE Volume = 0;";
    const std::string positiondetailUpdateAfterSettlementSql2
        = "UPDATE CThostFtdcInvestorPositionDetailField \
SET TradingDay = '" + newTradingDay + "', LastSettlementPrice = SettlementPrice, \
CloseProfitByDate = 0, CloseProfitByTrade = 0, PositionProfitByDate = 0, \
CloseVolume = 0, CloseAmount = 0;";
    const std::string positiondetailUpdateAfterSettlementSql3
        = "DELETE FROM CThostFtdcInvestorPositionDetailField \
 WHERE CThostFtdcInvestorPositionDetailField.InstrumentID in \
   (SELECT CThostFtdcInstrumentField.InstrumentID from CThostFtdcInstrumentField \
 WHERE CThostFtdcInstrumentField.ExpireDate <= '" + oldTradingDay +"');";
    m_sqlHandler.Delete(positiondetailUpdateAfterSettlementSql1);
    m_sqlHandler.Update(positiondetailUpdateAfterSettlementSql2);
    m_sqlHandler.Delete(positiondetailUpdateAfterSettlementSql3);

    //结算后更新所有账户的持仓
    //1. 删除持仓数量为0的持仓
    //2. "交易日"改为新的交易日
    //3. 修改"上次结算价"为与结算价相等, 并根据结算价计算新的持仓成本
    //4. "逐日平仓盈亏"与"逐笔平仓盈亏"与"持仓盈亏"改为0
    //5. 冻结持仓与冻结持仓金额与组合冻结持仓改为0
    //6. 冻结资金,冻结保证金,冻结手续费改为0
    //7. "上日持仓"改为与持仓数量相等
    //8. "开仓量"与"平仓量"与相应的金额改为0
    //9. "手续费"改为0
    //10. TodayPosition(今日持仓)改为0
    //11. 删除合约到期的合约的持仓.
    const std::string positionUpdateSqlAfterSettlementSql1 =
        "DELETE FROM CThostFtdcInvestorPositionField WHERE Position = 0;";

    const std::string positionUpdateSqlAfterSettlementSql2 =
        "UPDATE CThostFtdcInvestorPositionField SET TradingDay = '" + newTradingDay
        + "', PreSettlementPrice = SettlementPrice, \
PositionCost = SettlementPrice * Position * \
(SELECT VolumeMultiple from CThostFtdcInstrumentField WHERE \
 CThostFtdcInstrumentField.InstrumentID=CThostFtdcInvestorPositionField.InstrumentID), \
CloseProfit = 0, PositionProfit = 0, CloseProfitByDate = 0, CloseProfitByTrade = 0, \
LongFrozen = 0, ShortFrozen = 0, LongFrozenAmount = 0, ShortFrozenAmount = 0, \
FrozenMargin = 0, FrozenCash = 0, FrozenCommission = 0, YdPosition = Position, \
OpenVolume = 0, CloseVolume = 0, OpenAmount = 0, CloseAmount = 0, \
Commission = 0, TodayPosition = 0, CombLongFrozen = 0, CombShortFrozen = 0;";
    const std::string positionUpdateSqlAfterSettlementSql3
        = "DELETE FROM CThostFtdcInvestorPositionField \
 WHERE CThostFtdcInvestorPositionField.InstrumentID in \
   (SELECT CThostFtdcInstrumentField.InstrumentID from CThostFtdcInstrumentField \
 WHERE CThostFtdcInstrumentField.ExpireDate <= '" + oldTradingDay + "');";
    m_sqlHandler.Delete(positionUpdateSqlAfterSettlementSql1);
    m_sqlHandler.Update(positionUpdateSqlAfterSettlementSql2);
    m_sqlHandler.Delete(positionUpdateSqlAfterSettlementSql3);
    //结算后更新所有账户的资金
    //1. "交易日"改为新的交易日
    //2. "平仓盈亏"与"持仓盈亏"改为0
    //3. "手续费"改为0
    //4. 静态权益(PreBalance)改为与Balance相等
    //5. PreMargin改为与CurrMargin相等
    //6. 入金与出金改为0
    const std::string tradingAccountUpdateSqlAfterSettlement
        = "UPDATE CThostFtdcTradingAccountField SET \
TradingDay = '" + newTradingDay + "', CloseProfit = 0, PositionProfit = 0, \
Commission = 0, PreBalance = Balance, PreMargin = CurrMargin, \
Deposit = 0, Withdraw = 0;";
    m_sqlHandler.Update(tradingAccountUpdateSqlAfterSettlement);
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
        UseMargin += pos.UseMargin;
        SettlementPrice = pos.SettlementPrice;
        PreSettlementPrice = pos.PreSettlementPrice;
    }
};

void CLocalTraderApi::CSettlementHandler::doUserSettlement(
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
        sprintf(ch_single_line, format_settlement_account_summary14.c_str(), 0.0, 100 * (LEZ(data.Balance) ? 1.0 : data.CurrMargin) / data.Balance);
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_account_summary15.c_str(), 0.0, (LTZ(data.Available) ? (0 - data.Available) : 0));
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
                trade.TradeID);
            ostrstr << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_trade_end1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_trade_end2.c_str(), (int)tradeSqlValues.size(), total_Lots, total_Turnover, total_Fee, total_CloseProfit);
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
                closeDetail.CloseProfit);
            ostrstr << ch_single_line << "\r\n";
        }

        sprintf(ch_single_line, format_settlement_position_closed_end1.c_str());
        ostrstr << ch_single_line << "\r\n";
        sprintf(ch_single_line, format_settlement_position_closed_end2.c_str(), (int)closeDetailSqlValues.size(), total_position_closed_lots, total_position_closed_close_profit);
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
        const std::string selectPosDetailSql =
            CThostFtdcInvestorPositionDetailFieldWrapper::generateSelectSqlByUserID(brokerID, userID);
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
        const std::string selectPosSql =
            CThostFtdcInvestorPositionFieldWrapper::generateSelectSqlByUserID(brokerID, userID);
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
