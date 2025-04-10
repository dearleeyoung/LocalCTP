﻿#include "stdafx.h"
#include "LocalTraderApi.h"
#include "Properties.h"
#include <iostream>

using namespace localCTP;

#define CHECK_LOGIN(p, memberName) \
    if(p==nullptr) return -1; \
    if(!m_logined) return -1; \
    if(m_userID != p->memberName || m_brokerID != p->BrokerID) return -1;

#define CHECK_LOGIN_USER(p) CHECK_LOGIN(p, UserID)

#define CHECK_LOGIN_INVESTOR(p) CHECK_LOGIN(p, InvestorID)

#define CHECK_LOGIN_ACCOUNT(p) CHECK_LOGIN(p, AccountID)

#define CREATE_SQL_TABLE(tableName) sqlHandler.CreateTable(tableName##Wrapper::CREATE_TABLE_SQL, #tableName);


LocalCTPConfig getParamsFromConfig()
{
    static bool isFirstTimeLoad(true);
    static LocalCTPConfig ret;
    if (isFirstTimeLoad)
    {
        auto loadConfigFile = [&] {
            std::ifstream ifs("localctp.config");
            Properties prop;
            prop.loadProperties(ifs, '=', false);

            int running_mode = prop.getValue("running_mode", 0);
            if (running_mode == 0)
            {
                ret.running_mode = RUNNING_MODE::REALTIME_MODE;
            }
            else if (running_mode == 1)
            {
                ret.running_mode = RUNNING_MODE::BACKTEST_MODE;
            }
            else
            {
                ret.running_mode = RUNNING_MODE::REALTIME_MODE;
            }

            std::string backtest_startdate = prop.getValue("backtest_startdate", std::string());
            if (!backtest_startdate.empty())
            {
                ret.backtest_startdate.SetDateTime(
                    std::stoi(backtest_startdate.substr(0, 4)),//2025 in "20250326"
                    std::stoi(backtest_startdate.substr(4, 2)),//3 in "20250326"
                    std::stoi(backtest_startdate.substr(6, 2)),//26 in "20250326"
                    0,
                    0,
                    0
                );
            }

            int exit_after_settlement = prop.getValue("exit_after_settlement", 0);
            if (exit_after_settlement == 0)
            {
                ret.exit_after_settlement = false;
            }
            else
            {
                ret.exit_after_settlement = true;
            }

            std::string settlement_time = prop.getValue("settlement_time", std::string("17:00:00"));
            if (settlement_time.size() != 8)
            {
                std::cerr << "length of settlement_time param should be 8, for example: 17:00:00" << std::endl;
                std::exit(1);
            }
            ret.settlement_time = settlement_time;

            std::cout << "[LocalCTP] Load local config file, running_mode:" << running_mode
                << "(" << ret.running_mode << ")"
                << ", backtest_startdate:" << backtest_startdate
                << ", exit_after_settlement:" << exit_after_settlement
                << ", settlement_time:" << settlement_time
                << std::endl;
            if (RUNNING_MODE::BACKTEST_MODE == ret.running_mode)
            {
                std::cout << "[LocalCTP] Note: You are in " << RUNNING_MODE::BACKTEST_MODE
                    << ", it will delete all account data in database at the beginning!"
                    << " If you want to use " << RUNNING_MODE::REALTIME_MODE
                    << ", set running_mode=0 in config file (localctp.config)" << std::endl;
            }
        };
        loadConfigFile();

        isFirstTimeLoad = false;
    }
    return ret;
}

RUNNING_MODE getRunningModeFromConfig()
{
    return getParamsFromConfig().running_mode;
}
CLeeDateTime getDefaultTimeInBackTestModeFromConfig()
{
    return getParamsFromConfig().backtest_startdate;
}
bool getExitAfterSettlementFromConfig()
{
    return getParamsFromConfig().exit_after_settlement;
}
std::string getSettlementTimeFromConfig()
{
    return getParamsFromConfig().settlement_time;
}


std::set<CLocalTraderApi::SP_TRADE_API> CLocalTraderApi::trade_api_set;
std::atomic<int> CLocalTraderApi::maxSessionID(0);
std::map<std::string, long long> CLocalTraderApi::m_orderSysID; // 当前最大委托编号
std::map<std::string, long long> CLocalTraderApi::m_tradeID; // 当前最大成交编号
CLocalTraderApi::InstrMap CLocalTraderApi::m_instrData; //合约数据
std::map<std::string, CThostFtdcExchangeField> CLocalTraderApi::m_exchanges;// 交易所数据. key:交易所代码
std::map<std::string, CThostFtdcProductField> CLocalTraderApi::m_products;// 品种数据. key:品种代码
CSettlementHandler& CLocalTraderApi::settlementHandler =
    CSettlementHandler::getSettlementHandler( CLocalTraderApi::sqlHandler);
std::mutex CLocalTraderApi::m_mdMtx;
CLocalTraderApi::MarketDataMap CLocalTraderApi::m_mdData; //行情数据
const RUNNING_MODE CLocalTraderApi::m_runningMode = getRunningModeFromConfig();
const bool CLocalTraderApi::m_exitAfterSettlement = getExitAfterSettlementFromConfig();
const std::string CLocalTraderApi::m_settlementTime = getSettlementTimeFromConfig();
const CLeeDateTime CLocalTraderApi::m_defaultTimeInBackTestMode = getDefaultTimeInBackTestModeFromConfig();
CLeeDateTime CLocalTraderApi::m_latestMarketTime;//行情中最新的时间
CSqliteHandler CLocalTraderApi::sqlHandler("LocalCTP.db", {
    "CThostFtdcInvestorPositionField", "CThostFtdcInvestorPositionDetailField",  "CThostFtdcOrderField",
    "CThostFtdcTradeField", "CThostFtdcTradingAccountField", "CThostFtdcInstrumentField",
    "CThostFtdcInstrumentMarginRateField", "CThostFtdcInstrumentCommissionRateField",
    "CloseDetail", "SettlementData"
    });
const long long CLocalTraderApi::initStartTime = CLeeDateTime::GetCurrentTime().Get_time_t() * 1000 +
     CLeeDateTime::GetCurrentTime().GetMillisecond(); // 1612345678 999
std::string CLocalTraderApi::tradingDay;

void CLocalTraderApi::GetSingleContractFromCombinationContract(const std::string& CombinationContractID,
    std::vector<std::string>& SingleContracts)
{
    SingleContracts.clear();
    std::string::size_type blank_index = CombinationContractID.find(' ');
    if (blank_index == std::string::npos)//若没有找到空格,则以整个合约作为一个单腿合约.
    {
        SingleContracts.push_back(CombinationContractID);
        return;
    }
    std::string::size_type and_index = blank_index;
    std::string::size_type old_and_index = and_index;
    while (and_index != std::string::npos && old_and_index + 1 != CombinationContractID.size())
    {
        and_index = CombinationContractID.find('&', old_and_index + 1);//若没有再找到'&',则也需要将这最后一腿的合约代码保存
        const std::string _single_contract = CombinationContractID.substr(
            old_and_index + 1, and_index - (old_and_index + 1));
        if (!_single_contract.empty())
        {
            SingleContracts.push_back(_single_contract);
        }
        old_and_index = and_index;
    }
    return;
}


// 判断是否满足成交条件.
bool CLocalTraderApi::isMatchTrade(TThostFtdcDirectionType direction, double orderPrice,
    const MarketDataVec& mdVec, TradePriceVec& tradePriceVec)
{
    if (mdVec.empty())
    {
        return false;
    }
    else //单个独立的合约,或组合合约
    {
        double priceDiff(0);
        // 单个合约示例: rb2405. 卖出报单价3671元.
        // rb2405 买一价3670元,卖一价3672元.
        // 则卖出对应的合约对手价是: 3670元. 3671>3670,无法成交.
        //
        // 组合合约示例: m2401-m2405 组合合约.买入报单价480元.
        // m2401 买一价3998元,卖一价4000元.
        // m2405 买一价3500元,卖一价3505元.
        // 则买入对应的组合合约对手价(卖一价差)是: 4000-3500=500元. 500>480,无法成交.
        for (std::size_t legNo = 0; legNo < mdVec.size(); ++legNo)
        {
            auto directionT = (legNo % 2 == 0 ?
                direction : getOppositeDirection(direction));
            int multi = (legNo % 2 == 0 ? 1 : -1);
            double legTradePrice(0);
            if (directionT == THOST_FTDC_D_Buy)
            {
                legTradePrice = mdVec[legNo].AskPrice1;
                priceDiff += legTradePrice * multi;
            }
            else
            {
                legTradePrice = mdVec[legNo].BidPrice1;
                priceDiff += legTradePrice * multi;
            }
            tradePriceVec.emplace_back(legTradePrice);
        }
        return ((direction == THOST_FTDC_D_Buy && GE(orderPrice, priceDiff)) ||
            (direction == THOST_FTDC_D_Sell && LE(orderPrice, priceDiff)));
    }
}

CLocalTraderApi::CLocalTraderApi(const char *pszFlowPath/* = ""*/)
	: m_bRunning(false), m_authenticated(false), m_logined(false)
    , m_frontID(static_cast<int>(CLeeDateTime::GetCurrentTime().Get_time_t())), m_sessionID(0)
    , m_tradingAccount{ 0 }, m_messageQueue()
    , m_successRspInfo{ 0, "success" }, m_errorRspInfo{ -1, "error" }
{
    m_tradingAccount.PreBalance = 2e7;
    m_tradingAccount.Balance = 2e7;



#ifdef _DEBUG
    std::cout << "[LocalCTP] Welcome to LocalCTP!" << std::endl;
#endif
}

CLocalTraderApi::~CLocalTraderApi()
{
	m_bRunning = false;
}

CThostFtdcRspInfoField* CLocalTraderApi::setErrorMsgAndGetRspInfo(const char* errorMsg /*= "error"*/)
{
    strncpy(m_errorRspInfo.ErrorMsg, errorMsg, sizeof(m_errorRspInfo.ErrorMsg));
    return &m_errorRspInfo;
}

// 收到行情快照的处理
// 接收到行情快照时，需要更新行情的合约对应的持仓的持仓盈亏，然后更新账户的动态权益等资金数据。
// 多头持仓的持仓盈亏 = （最新结算价计算得到的成本 - 持仓成本） * 合约数量乘数 * 持仓数量
// 空头持仓的持仓盈亏 = （持仓成本 - 最新结算价计算得到的成本） * 合约数量乘数 * 持仓数量
//
// 注: 收到行情快照时, 会更新持仓和资金并写入到(内存)数据库中, 如果收取的行情快照很多很频繁, 则可能导致写入频繁而卡顿,
// 本系统目前是收到每次快照时实时写入, 是为了使持仓和资金等维护准确, 即确保内存和数据库中的数据的一致性.
// 用户可根据需要修改写入数据库的逻辑来避免卡顿的情况, 例如改为定时写入等.
void CLocalTraderApi::onSnapshot(const CThostFtdcDepthMarketDataField& mdData)
{
    const std::string instrumentID = mdData.InstrumentID;
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        m_mdData[instrumentID] = mdData;
    }
    {
        //更新回测模式中的当前时间
        const std::string updateTimeStr = mdData.UpdateTime; //"21:55:59"
        if (updateTimeStr.size() < 8)
        {
            return;
        }
        CLeeDateTime updateTime;
        if (std::stoi(updateTimeStr.substr(0, 2)) >= 20) //夜盘行情时间
        {
            const std::string minDayStr = (std::min)(std::string(mdData.ActionDay), std::string(mdData.TradingDay)); //"20250313"
            if (tradingDay.empty() || minDayStr.size() < 8) return;
            updateTime.SetDateTime(std::stoi(minDayStr.substr(0, 4)),
                std::stoi(minDayStr.substr(4, 2)),
                std::stoi(minDayStr.substr(6, 2)),
                std::stoi(updateTimeStr.substr(0, 2)),
                std::stoi(updateTimeStr.substr(3, 2)),
                std::stoi(updateTimeStr.substr(6, 2)),
                mdData.UpdateMillisec);
            if (minDayStr < tradingDay)
            {
            }
            else //说明 ActionDay和TradingDay 的较小者仍不为当前实际日期, 则需要求出交易日的前一天作为当前实际日期
            {
                while (!isTradingDay(updateTime))
                {
                    updateTime -= CLeeDateTimeSpan(1, 0, 0, 0);
                }
            }
        }
        else
        {
            updateTime.SetDateTime(std::stoi(tradingDay.substr(0, 4)),
                std::stoi(tradingDay.substr(4, 2)),
                std::stoi(tradingDay.substr(6, 2)),
                std::stoi(updateTimeStr.substr(0, 2)),
                std::stoi(updateTimeStr.substr(3, 2)),
                std::stoi(updateTimeStr.substr(6, 2)),
                mdData.UpdateMillisec);
        }
        if (updateTime > m_latestMarketTime)
        {
            m_latestMarketTime = updateTime;
        }
    }

    auto it = m_instrData.find(instrumentID);
    if (it == m_instrData.end())
    {
        return;
    }

    auto isPriceValid = [](const double price) ->bool {return LT(price, DBL_MAX) && NEZ(price); };
    //处理条件单的触发
    for (auto contionalOrderPtr : m_contionalOrders)
    {
        auto& contionalOrder = *contionalOrderPtr;
        if (instrumentID != contionalOrder.rtnOrder.InstrumentID
            || contionalOrder.rtnOrder.OrderStatus == THOST_FTDC_OST_Touched)
        {
            continue;
        }
        auto matchContion = [&]() -> bool {
            switch (contionalOrder.rtnOrder.ContingentCondition)
            {
            case THOST_FTDC_CC_LastPriceGreaterThanStopPrice:
                return isPriceValid(mdData.LastPrice) && GT(mdData.LastPrice, contionalOrder.rtnOrder.StopPrice);
            case THOST_FTDC_CC_LastPriceGreaterEqualStopPrice:
                return isPriceValid(mdData.LastPrice) && GE(mdData.LastPrice, contionalOrder.rtnOrder.StopPrice);
            case THOST_FTDC_CC_LastPriceLesserThanStopPrice:
                return isPriceValid(mdData.LastPrice) && LT(mdData.LastPrice, contionalOrder.rtnOrder.StopPrice);
            case THOST_FTDC_CC_LastPriceLesserEqualStopPrice:
                return isPriceValid(mdData.LastPrice) && LE(mdData.LastPrice, contionalOrder.rtnOrder.StopPrice);
            default:
                return false;
            }
        };
        if (!matchContion())
        {
            continue;
        }
        contionalOrder.rtnOrder.OrderStatus = THOST_FTDC_OST_Touched;
        strncpy(contionalOrder.rtnOrder.StatusMsg,
            getStatusMsgByStatus(contionalOrder.rtnOrder.OrderStatus).c_str(),
            sizeof(contionalOrder.rtnOrder.StatusMsg));
        contionalOrder.sendRtnOrder();

        // send a new non-conditional order
        CThostFtdcInputOrderField InputOrder = contionalOrder.inputOrder;
        strncpy(InputOrder.OrderRef, "", sizeof(InputOrder.OrderRef));
        InputOrder.LimitPrice = contionalOrder.inputOrder.StopPrice;
        InputOrder.StopPrice = 0;
        InputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;

        ReqOrderInsertImpl(&InputOrder, 0, contionalOrder.rtnOrder.OrderSysID);
    }

    // 不根据组合合约行情快照来更新PNL和报单,因此这里直接返回
    if (it->second.ProductClass == THOST_FTDC_PC_Combination)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        // 此时订单数据中已包含刚才条件单触发后产生的新报单O(∩_∩)O
        for (auto& sessionOrders : m_orderData)
        {
            for (auto& o : sessionOrders.second)
            {
                auto& order = o.second;
                // 可能是组合合约的报单,合约代码与行情中的合约代码不同
                if (std::string(order.rtnOrder.InstrumentID).find(instrumentID) == std::string::npos ||
                    order.isDone())
                {
                    continue;
                }
                MarketDataVec mdVec;
                if (order.rtnOrder.InstrumentID == instrumentID)//合约代码相等,说明是单腿合约
                {
                    mdVec.emplace_back(mdData);
                }
                else
                {
                    std::vector<std::string> singleContracts;
                    GetSingleContractFromCombinationContract(order.rtnOrder.InstrumentID, singleContracts);

                    for (std::size_t legNo = 0; legNo < singleContracts.size(); ++legNo)
                    {
                        const std::string instr = singleContracts[legNo];

                        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
                        auto itDepthMarketData = m_mdData.find(instr);
                        if (itDepthMarketData == m_mdData.end())
                        {
                            continue;
                        }
                        mdVec.emplace_back(itDepthMarketData->second);
                    }
                }
                TradePriceVec priceVec;
                // 对组合合约,判断两条腿的差价,成交价格使用两条腿各自的盘口价格,而并不使用组合合约自身的行情,
                if (isMatchTrade(order.rtnOrder.Direction, order.rtnOrder.LimitPrice, mdVec, priceVec))
                {
                    order.handleTrade(priceVec, order.rtnOrder.VolumeTotal);
                }
            }
        }
    }
    if (!isPriceValid(mdData.LastPrice) && !isPriceValid(mdData.SettlementPrice))
    {
        return;
    }
    const double priceForCal = isPriceValid(mdData.SettlementPrice) ?
        mdData.SettlementPrice : mdData.LastPrice;//优先使用结算价进行计算
    double diffPositionProfit(0);
    CSqliteTransactionHandler transactionHandle(sqlHandler);
    static size_t mdDataCounter = 0;
    bool shouldUpdateSql = true;
    if (CLocalTraderApi::m_runningMode == RUNNING_MODE::BACKTEST_MODE)
    {
        if (++mdDataCounter >= 100)
        {
            mdDataCounter = 0;
            shouldUpdateSql = true;
        }
        else
        {
            shouldUpdateSql = false;
        }
    }
    for (auto dir : { THOST_FTDC_D_Buy, THOST_FTDC_D_Sell })
    {
        for (auto dateType : { THOST_FTDC_PSD_Today, THOST_FTDC_PSD_History })
        {
            auto posKey = CLocalTraderApi::generatePositionKey(instrumentID,
                dir, dateType);
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            // 更新持仓中的PNL
            auto itPos = m_positionData.find(posKey);
            if (itPos != m_positionData.end())
            {
                for (auto& posDetail : itPos->second.posDetailData)
                {
                    //行情没变化则不更新
                    if (EQ(posDetail.SettlementPrice, priceForCal))
                    {
                        continue;
                    }
                    posDetail.SettlementPrice = priceForCal;
                    if (isOptions(it->second.ProductClass))
                    {
                        //期权不计算持仓盈亏
                    }
                    else
                    {
                        const double positionCostOfThiPosDetail =
                            (strcmp(posDetail.OpenDate, posDetail.TradingDay) == 0 ?
                                posDetail.OpenPrice : posDetail.LastSettlementPrice)
                            * posDetail.Volume * it->second.VolumeMultiple;
                        const double openCostOfThiPosDetail =
                            posDetail.OpenPrice * posDetail.Volume * it->second.VolumeMultiple;
                        posDetail.PositionProfitByTrade = (dir == THOST_FTDC_D_Buy ? 1 : -1) *
                            (priceForCal * it->second.VolumeMultiple * posDetail.Volume
                                - openCostOfThiPosDetail);
                        posDetail.PositionProfitByDate = (dir == THOST_FTDC_D_Buy ? 1 : -1) *
                            (priceForCal * it->second.VolumeMultiple * posDetail.Volume
                                - positionCostOfThiPosDetail);
                    }

                    // only update some fields in position detail table
                    const std::string updatePositionDetailProfitSql =
                        std::string("UPDATE 'CThostFtdcInvestorPositionDetailField' SET PositionProfitByTrade=")
                        + std::to_string(posDetail.PositionProfitByTrade)
                        + ", PositionProfitByDate=" + std::to_string(posDetail.PositionProfitByDate)
                        + ", SettlementPrice=" + std::to_string(posDetail.SettlementPrice)
                        + " WHERE BrokerID='" + posDetail.BrokerID
                        + "' AND InvestorID='" + posDetail.InvestorID
                        + "' AND HedgeFlag='" + posDetail.HedgeFlag
                        + "' AND InstrumentID='" + posDetail.InstrumentID
                        + "' AND Direction='" + posDetail.Direction
                        + "' AND TradeType='" + posDetail.TradeType
                        + "' AND OpenDate='" + posDetail.OpenDate
                        + "' AND TradeID='" + posDetail.TradeID
                        + "';";
                    if (shouldUpdateSql)
                    {
                        sqlHandler.Update(updatePositionDetailProfitSql);
                    }
                }

                //行情没变化则不更新
                if (EQ(itPos->second.pos.SettlementPrice, priceForCal))
                {
                    continue;
                }
                itPos->second.pos.SettlementPrice = priceForCal;
                if (isOptions(it->second.ProductClass))
                {
                    //期权不计算持仓盈亏
                }
                else
                {
                    auto& PositionProfit = itPos->second.pos.PositionProfit;
                    double oldPositionProfit = PositionProfit;
                    PositionProfit = (dir == THOST_FTDC_D_Buy ? 1 : -1) *
                        (priceForCal * it->second.VolumeMultiple * itPos->second.pos.Position
                            - itPos->second.pos.PositionCost);
                    diffPositionProfit += PositionProfit - oldPositionProfit;
                }

                // only update some fields in position table
                const std::string updatePositionProfitSql =
                    std::string("UPDATE 'CThostFtdcInvestorPositionField' SET PositionProfit=")
                    + std::to_string(itPos->second.pos.PositionProfit)
                    + ", SettlementPrice=" + std::to_string(itPos->second.pos.SettlementPrice)
                    + " WHERE BrokerID='" + itPos->second.pos.BrokerID
                    + "' AND InvestorID='" + itPos->second.pos.InvestorID
                    + "' AND HedgeFlag='" + itPos->second.pos.HedgeFlag
                    + "' AND InstrumentID='" + itPos->second.pos.InstrumentID
                    + "' AND PosiDirection='" + itPos->second.pos.PosiDirection
                    + "' AND PositionDate='" + itPos->second.pos.PositionDate + "';";
                if (shouldUpdateSql)
                {
                    sqlHandler.Update(updatePositionProfitSql);
                }
            }
        }
    }
    m_tradingAccount.PositionProfit += diffPositionProfit;
    if (NEZ(diffPositionProfit))
    {
        updatePNL(false, shouldUpdateSql);
    }
}


// 计算PNL(profit and loss, 盈亏)
void CLocalTraderApi::updatePNL(bool needTotalCalc /*= false*/, bool shouldUpdateSql /*= true*/)
{
    if (needTotalCalc)
    {
        m_tradingAccount.PositionProfit = 0;
        m_tradingAccount.CloseProfit = 0;
        m_tradingAccount.Commission = 0;
        m_tradingAccount.CurrMargin = 0;
        m_tradingAccount.FrozenMargin = 0;
        m_tradingAccount.FrozenCash = 0;
        m_tradingAccount.CashIn = 0;
        for (const auto& P : m_positionData)
        {
            m_tradingAccount.PositionProfit += P.second.pos.PositionProfit;
            m_tradingAccount.CloseProfit += P.second.pos.CloseProfit;
            m_tradingAccount.Commission += P.second.pos.Commission;
            m_tradingAccount.CurrMargin += P.second.pos.UseMargin;
            m_tradingAccount.FrozenMargin += P.second.pos.FrozenMargin;
            m_tradingAccount.FrozenCash += P.second.pos.FrozenCash;
            m_tradingAccount.CashIn += P.second.pos.CashIn;
        }
    }
    m_tradingAccount.Balance = m_tradingAccount.PreBalance
        + m_tradingAccount.Deposit - m_tradingAccount.Withdraw
        + m_tradingAccount.PositionProfit + m_tradingAccount.CloseProfit
        + m_tradingAccount.CashIn
        - m_tradingAccount.Commission;
    m_tradingAccount.Available = m_tradingAccount.Balance
        - m_tradingAccount.CurrMargin - m_tradingAccount.FrozenMargin
        - m_tradingAccount.FrozenCash;

    // PNL更新时保存资金数据到数据库中. 可根据需要修改控制保存的时机(如定时保存等).
    if (shouldUpdateSql)
    {
        saveTradingAccountToDb();
    }
}

void CLocalTraderApi::updateByCancel(const CThostFtdcOrderField& o)
{
    if (THOST_FTDC_OST_Canceled != o.OrderStatus) return;

    auto it = m_instrData.find(o.InstrumentID);
    if (it == m_instrData.end())
    {
        return;
    }
    std::string instr = o.InstrumentID;
    auto directionT = o.Direction;

    auto handleOpen = [&]()
    {
        MarketDataMap::iterator itDepthMarketData;
        {
            std::lock_guard<std::mutex> mdGuard(m_mdMtx);
            itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return;
            }
        }
        auto posKey = generatePositionKey(instr,
            directionT,
            THOST_FTDC_PSD_Today);
        PositionDataMap::iterator itPos;
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                return;
            }
        }
        double frozenMargin(0);
        double frozenCash(0);
        if (isOptions(it->second.ProductClass))
        {
            if (directionT == THOST_FTDC_D_Buy)//期权买入的委托, 需解冻一部分权利金
            {
                frozenCash = itDepthMarketData->second.PreSettlementPrice *
                    it->second.VolumeMultiple * o.VolumeTotal;
            }
            else//期权卖出的(开仓)委托, 需解冻一部分保证金(期权保证金计算太复杂啦,本系统以期货保证金计算规则来算期权保证金)
            {
            }
        }
        frozenMargin = itDepthMarketData->second.PreSettlementPrice *
            it->second.VolumeMultiple * o.VolumeTotal *
            itPos->second.pos.MarginRateByMoney
            + o.VolumeTotal * itPos->second.pos.MarginRateByVolume;
        itPos->second.pos.FrozenMargin = (std::max)(
            itPos->second.pos.FrozenMargin - frozenMargin, 0.0);
        itPos->second.pos.FrozenCash = (std::max)(
            itPos->second.pos.FrozenCash - frozenCash, 0.0);

        m_tradingAccount.FrozenMargin = (std::max)(
            m_tradingAccount.FrozenMargin - frozenMargin, 0.0);
        m_tradingAccount.FrozenCash = (std::max)(
            m_tradingAccount.FrozenCash - frozenCash, 0.0);
        if (NEZ(frozenMargin) || NEZ(frozenCash))
        {
            CSqliteTransactionHandler transactionHandle(sqlHandler);
            savePositionToDb(itPos->second.pos);
            updatePNL();
        }
    };
    auto handleClose = [&]()
    {
        MarketDataMap::iterator itDepthMarketData;
        {
            std::lock_guard<std::mutex> mdGuard(m_mdMtx);
            itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return;
            }
        }
        auto posKey = generatePositionKey(instr,
            directionT,
            getDateTypeFromOffset(
                it->second.ExchangeID, o.CombOffsetFlag[0]));
        PositionDataMap::iterator itPos;
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                // 如果没找到持仓,则返回,并不插入持仓(因为持仓在开仓报单时已插入).此流程可改进
                return;
            }
        }
        int& frozenPositionNum = (itPos->second.pos.PosiDirection == THOST_FTDC_PD_Long ?
            itPos->second.pos.ShortFrozen : itPos->second.pos.LongFrozen);
        frozenPositionNum = (std::max)(
            frozenPositionNum - o.VolumeTotal, 0);
        double frozenCash(0);
        if (isOptions(it->second.ProductClass))
        {
            if (getOppositeDirection(directionT) == THOST_FTDC_D_Buy)//期权买入的委托, 需解冻一部分权利金
            {
                frozenCash = itDepthMarketData->second.PreSettlementPrice *
                    it->second.VolumeMultiple * o.VolumeTotal;
            }
        }
        itPos->second.pos.FrozenCash = (std::max)(
            itPos->second.pos.FrozenCash - frozenCash, 0.0);
        CSqliteTransactionHandler transactionHandle(sqlHandler);
        savePositionToDb(itPos->second.pos);
        m_tradingAccount.FrozenCash = (std::max)(
            m_tradingAccount.FrozenCash - frozenCash, 0.0);
        if (NEZ(frozenCash))
        {
            updatePNL();
        }
    };
    // 开仓报单已撤单,则减少冻结保证金(对应数量是未成交数量)
    if (isOpen(o.CombOffsetFlag[0]))
    {
        instr = o.InstrumentID;
        directionT = o.Direction;

        if (it->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            std::vector<std::string> singleContracts;
            GetSingleContractFromCombinationContract(it->first, singleContracts);

            for (std::size_t legNo = 0; legNo < singleContracts.size(); ++legNo)
            {
                instr = singleContracts[legNo];
                directionT = (legNo % 2 == 0 ?
                    o.Direction
                    : getOppositeDirection(o.Direction));

                ((o.IsSwapOrder != 0 && legNo % 2 != 0) ?
                    handleClose() : handleOpen());//需考虑互换单的情况
            }
        }
        else
        {
            handleOpen();
        }
    }
    else// 平仓报单已撤单,则减少冻结持仓(对应数量是未成交数量)
    {
        instr = o.InstrumentID;
        directionT = getOppositeDirection(o.Direction);

        if (it->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            std::vector<std::string> singleContracts;
            GetSingleContractFromCombinationContract(it->first, singleContracts);

            for (std::size_t legNo = 0; legNo < singleContracts.size(); ++legNo)
            {
                instr = singleContracts[legNo];
                directionT = (legNo % 2 == 0 ?
                    getOppositeDirection(o.Direction)
                    : o.Direction);

                ((o.IsSwapOrder != 0 && legNo % 2 != 0) ?
                    handleOpen() : handleClose());//需考虑互换单的情况
            }
        }
        else
        {
            handleClose();
        }
    }
}

void CLocalTraderApi::updateByTrade(const CThostFtdcTradeFieldWrapper& t)
{
    const auto* pTrade = &(t.data);

    auto it = m_instrData.find(pTrade->InstrumentID);
    if (it == m_instrData.end())
    {
        return;
    }
    auto itCommissionRate = m_instrumentCommissionRateData.find(pTrade->InstrumentID);
    if (itCommissionRate == m_instrumentCommissionRateData.end())
    {
        return;
    }

    MarketDataMap::iterator itDepthMarketData;
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        itDepthMarketData = m_mdData.find(pTrade->InstrumentID);
        if (itDepthMarketData == m_mdData.end())
        {
            return;
        }
    }
    double cashIn(0);//权利金收支
    double frozenCash(0);//冻结的权利金
    if (isOptions(it->second.ProductClass))
    {
        if (pTrade->Direction == THOST_FTDC_D_Buy)//期权买入的委托, 需解冻一部分权利金
        {
            frozenCash = itDepthMarketData->second.PreSettlementPrice *
                it->second.VolumeMultiple * pTrade->Volume;
            cashIn = -1 * pTrade->Price * pTrade->Volume * it->second.VolumeMultiple;
        }
        else
        {
            cashIn = 1 * pTrade->Price * pTrade->Volume * it->second.VolumeMultiple;
        }
    }
    // 开仓成交时,增加一笔新的持仓明细,减少冻结保证金,更新持仓和资金
    if (isOpen(pTrade->OffsetFlag))
    {
        const double feeOfTrade = pTrade->Price * it->second.VolumeMultiple *
            pTrade->Volume * itCommissionRate->second.OpenRatioByMoney +
            pTrade->Volume * itCommissionRate->second.OpenRatioByVolume;
        auto& tNonConst = const_cast<CThostFtdcTradeFieldWrapper&>(t);
        tNonConst.Commission = feeOfTrade;
        tNonConst.CashIn = cashIn;

        auto posKey = generatePositionKey(pTrade->InstrumentID,
            pTrade->Direction,
            THOST_FTDC_PSD_Today);
        PositionDataMap::iterator itPos;
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                // 如果没找到持仓,则返回,并不插入持仓(因为持仓在开仓报单时已插入).
                return;
            }
        }

        const double frozenMargin = itDepthMarketData->second.PreSettlementPrice *
            it->second.VolumeMultiple * pTrade->Volume * itPos->second.pos.MarginRateByMoney
            + pTrade->Volume * itPos->second.pos.MarginRateByVolume;
        const double marginOfTrade = pTrade->Price * it->second.VolumeMultiple *
            pTrade->Volume * itPos->second.pos.MarginRateByMoney
            + pTrade->Volume * itPos->second.pos.MarginRateByVolume;
        {
            auto posDetail = PositionData::getPositionDetailFromOpenTrade(*pTrade);
            posDetail.MarginRateByMoney = itPos->second.pos.MarginRateByMoney;
            posDetail.MarginRateByVolume = itPos->second.pos.MarginRateByVolume;
            posDetail.LastSettlementPrice = itDepthMarketData->second.PreSettlementPrice;
            posDetail.SettlementPrice = itDepthMarketData->second.SettlementPrice;
            posDetail.Margin = marginOfTrade;
            itPos->second.addPositionDetail(posDetail);// 添加持仓明细到持仓中
            savePositionDetialToDb(posDetail);

            auto& pos = itPos->second.pos;
            pos.Position += pTrade->Volume;
            pos.Commission += feeOfTrade;
            pos.UseMargin += marginOfTrade;
            pos.FrozenMargin = (std::max)(
                pos.FrozenMargin - frozenMargin, 0.0);
            pos.FrozenCash = (std::max)(
                pos.FrozenCash - frozenCash, 0.0);//更新持仓的冻结权利金
            pos.CashIn += cashIn;//更新持仓的权利金收支
            pos.PositionCost +=
                pTrade->Price * pTrade->Volume * itPos->second.volumeMultiple;//更新持仓的持仓成本
            pos.OpenVolume += pTrade->Volume;
            pos.OpenAmount +=
                pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;
            pos.OpenCost +=
                pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;
            pos.TodayPosition += pTrade->Volume;
            savePositionToDb(pos);
        }
        // 汇总计算资金
        updatePNL(true);
    }
    // 平仓成交时,按"先开先平"的原则更新持仓明细,减少持仓中的冻结持仓,更新持仓和资金.
    else
    {
        auto posKey = generatePositionKey(pTrade->InstrumentID,
            getOppositeDirection(pTrade->Direction),
            getDateTypeFromOffset(pTrade->ExchangeID, pTrade->OffsetFlag));

        std::lock_guard<std::mutex> posGuard(m_positionMtx);
        auto itPos = m_positionData.find(posKey);
        if (itPos == m_positionData.end())
        {
            return;
        }
        else
        {
            int restVolume(pTrade->Volume);// 成交剩余的数量
            int closeYesterdayVolume(0);// 此次成交中的平昨数量
            int closeTodayVolume(0);// 此次成交中的平今数量
            double closeProfitOfTrade(0);// 此次成交的平仓盈亏
            double closeProfitByTradeOfTrade(0);// 此次成交的逐笔对冲平仓盈亏

            auto& pos = itPos->second.pos;

            for (auto& p : itPos->second.posDetailData)
            {
                if (p.Volume <= 0)
                    continue;
                int tradeVolumeInThisPosDetail(0);
                if (p.Volume >= restVolume)
                {
                    tradeVolumeInThisPosDetail = restVolume;
                    p.Volume -= tradeVolumeInThisPosDetail;
                    restVolume = 0;
                }
                else
                {
                    tradeVolumeInThisPosDetail = p.Volume;
                    restVolume -= tradeVolumeInThisPosDetail;
                    p.Volume = 0;
                }
                p.CloseVolume += tradeVolumeInThisPosDetail;
                p.CloseAmount +=
                    tradeVolumeInThisPosDetail * pTrade->Price * itPos->second.volumeMultiple;
                const double positionCostOfThiPosDetail =
                    (strcmp(p.OpenDate, p.TradingDay) == 0 ?
                        p.OpenPrice : p.LastSettlementPrice)
                    * p.Volume * it->second.VolumeMultiple;
                if (!isOptions(it->second.ProductClass))//期权无持仓盈亏
                {
                    const double openCostOfThiPosDetail =
                        p.OpenPrice * p.Volume * it->second.VolumeMultiple;
                    p.PositionProfitByTrade = (p.Direction == THOST_FTDC_D_Buy ? 1 : -1) *
                        (pTrade->Price * it->second.VolumeMultiple * p.Volume
                            - openCostOfThiPosDetail);//更新持仓明细的逐笔对冲持仓盈亏
                    p.PositionProfitByDate = (p.Direction == THOST_FTDC_D_Buy ? 1 : -1) *
                        (pTrade->Price * it->second.VolumeMultiple * p.Volume
                            - positionCostOfThiPosDetail);//更新持仓明细的逐日盯市持仓盈亏
                }
                p.Margin = positionCostOfThiPosDetail * pos.MarginRateByMoney
                    + p.Volume * pos.MarginRateByVolume;// 更新持仓明细的保证金
                // 持仓明细的平仓盈亏汇总计算中,昨仓用昨结算价,今仓用开仓价
                double closeProfitOfThisPosDetailInThisTrade = 0;
                TThostFtdcOffsetFlagType _closeFlag = THOST_FTDC_OF_Close;
                if (strcmp(p.OpenDate, p.TradingDay) != 0)//昨仓
                {
                    closeYesterdayVolume += tradeVolumeInThisPosDetail;
                    if (!isOptions(it->second.ProductClass))//期权无平仓盈亏
                    {
                        closeProfitOfThisPosDetailInThisTrade =
                            (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1) *
                            (pTrade->Price - pos.PreSettlementPrice) *
                            tradeVolumeInThisPosDetail * itPos->second.volumeMultiple;
                    }
                    _closeFlag = THOST_FTDC_OF_Close;
                }
                else//今仓
                {
                    closeTodayVolume += tradeVolumeInThisPosDetail;
                    if (!isOptions(it->second.ProductClass))//期权无平仓盈亏
                    {
                        closeProfitOfThisPosDetailInThisTrade =
                            (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1) *
                            (pTrade->Price - p.OpenPrice) *
                            tradeVolumeInThisPosDetail * itPos->second.volumeMultiple;
                    }
                    _closeFlag = THOST_FTDC_OF_CloseToday;
                }
                if (!isOptions(it->second.ProductClass))//期权无平仓盈亏
                {
                    double closeProfitByTradeOfThisPosDetailInThisTrade =
                        (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1) *
                        (pTrade->Price - p.OpenPrice) *
                        tradeVolumeInThisPosDetail * itPos->second.volumeMultiple;
                    p.CloseProfitByDate += closeProfitOfThisPosDetailInThisTrade;
                    p.CloseProfitByTrade += closeProfitByTradeOfThisPosDetailInThisTrade;
                    closeProfitOfTrade += closeProfitOfThisPosDetailInThisTrade;
                    closeProfitByTradeOfTrade += closeProfitByTradeOfThisPosDetailInThisTrade;
                }

                savePositionDetialToDb(p);
                if (tradeVolumeInThisPosDetail > 0)//此处实际不会为0,判断是为了保险
                {
                    CloseDetail cd = { 0 };
                    strncpy(cd.BrokerID, pTrade->BrokerID, sizeof(cd.BrokerID));
                    strncpy(cd.InvestorID, pTrade->InvestorID, sizeof(cd.InvestorID));
                    strncpy(cd.ExchangeID, pTrade->ExchangeID, sizeof(cd.ExchangeID));
                    strncpy(cd.InstrumentID, pTrade->InstrumentID, sizeof(cd.InstrumentID));
                    strncpy(cd.OpenDate, p.OpenDate, sizeof(cd.OpenDate));
                    cd.OpenPrice = p.OpenPrice;
                    strncpy(cd.OpenTradeID, p.TradeID, sizeof(cd.OpenTradeID));
                    strncpy(cd.CloseDate, pTrade->TradingDay, sizeof(cd.CloseDate));
                    strncpy(cd.CloseTime, pTrade->TradeTime, sizeof(cd.CloseTime));
                    cd.ClosePrice = pTrade->Price;
                    strncpy(cd.CloseTradeID, pTrade->TradeID, sizeof(cd.CloseTradeID));
                    cd.CloseVolume = tradeVolumeInThisPosDetail;
                    cd.Direction = pTrade->Direction;
                    cd.PreSettlementPrice = itDepthMarketData->second.PreSettlementPrice;
                    cd.CloseProfit = closeProfitOfThisPosDetailInThisTrade;
                    cd.CloseFlag = _closeFlag;
                    if (isOptions(it->second.ProductClass))
                    {
                        cd.CashIn = (pTrade->Direction == THOST_FTDC_D_Buy ? -1 : 1)
                            * pTrade->Price * it->second.VolumeMultiple * tradeVolumeInThisPosDetail;
                    }
                    saveDataToDb(CloseDetailWrapper(cd));
                }
                if (restVolume <= 0)
                {
                    break;
                }
            }

            double feeCloseYesterdayOfTrade = pTrade->Price * it->second.VolumeMultiple *
                closeYesterdayVolume * itCommissionRate->second.CloseRatioByMoney +
                closeYesterdayVolume * itCommissionRate->second.CloseRatioByVolume;
            double feeCloseTodayOfTrade = pTrade->Price * it->second.VolumeMultiple *
                closeTodayVolume * itCommissionRate->second.CloseTodayRatioByMoney +
                closeTodayVolume * itCommissionRate->second.CloseTodayRatioByVolume;
            double feeOfTrade = feeCloseYesterdayOfTrade + feeCloseTodayOfTrade;
            // set commission for this trade
            auto& tNonConst = const_cast<CThostFtdcTradeFieldWrapper&>(t);
            tNonConst.Commission = feeOfTrade;
            tNonConst.CloseProfit = closeProfitOfTrade;
            tNonConst.CashIn = cashIn;

            //重新统计持仓中的一些数据
            pos.Position = 0;// 持仓数量
            pos.PositionCost = 0;// 持仓成本
            pos.OpenCost = 0;// 开仓成本
            for (auto& p : itPos->second.posDetailData)
            {
                pos.Position += p.Volume;
                pos.PositionCost += p.Volume *
                    (strcmp(p.OpenDate, p.TradingDay)==0 ? p.OpenPrice : pos.PreSettlementPrice)
                    * itPos->second.volumeMultiple;// 持仓明细的持仓成本汇总计算中,昨仓用昨结算价,今仓用开仓价
                pos.OpenCost += p.Volume * p.OpenPrice * itPos->second.volumeMultiple;
            }
            if (!isOptions(it->second.ProductClass))//期权无持仓盈亏
            {
                pos.PositionProfit =
                    (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1)
                    * (pTrade->Price * itPos->second.volumeMultiple * pos.Position
                        - pos.PositionCost);// 持仓盈亏
            }
            pos.CloseProfit += closeProfitOfTrade;// 平仓盈亏
            pos.CloseProfitByDate += closeProfitOfTrade;// 逐日盯市平仓盈亏
            pos.CloseProfitByTrade += closeProfitByTradeOfTrade;// 逐笔对冲平仓盈亏
            pos.Commission += feeOfTrade;// 更新持仓的手续费
            pos.UseMargin =
                pos.PositionCost * pos.MarginRateByMoney
                + pos.Position * pos.MarginRateByVolume;// 更新持仓的保证金
            int& frozenPositionNum = (pos.PosiDirection == THOST_FTDC_PD_Long ?
                pos.ShortFrozen : pos.LongFrozen);
            frozenPositionNum = (std::max)(
                frozenPositionNum - pTrade->Volume, 0);//更新持仓的冻结持仓
            pos.FrozenCash = (std::max)(
                pos.FrozenCash - frozenCash, 0.0);//更新持仓的冻结权利金
            pos.CashIn += cashIn;//更新持仓的权利金收支
            pos.CloseVolume += pTrade->Volume;//更新持仓的平仓量
            pos.CloseAmount +=
                pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;//更新持仓的平仓金额
            pos.TodayPosition -= closeTodayVolume;
            savePositionToDb(pos);
            // 汇总计算资金
            updatePNL(true);
        }
    }
}

// 从数据库中重新加载账户数据
void CLocalTraderApi::reloadAccountData()
{
    if (!m_logined) return;
    const std::string& TradingDay = GetTradingDay();
    auto reloadMarginAndCommissionRate = [&]() {
        CSqliteHandler::SQL_VALUES marinRateSqlValues;
        sqlHandler.SelectData(CThostFtdcInstrumentMarginRateFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            marinRateSqlValues);
        for (const auto& rowData : marinRateSqlValues)
        {
            CThostFtdcInstrumentMarginRateFieldWrapper marginRate(rowData);
            m_instrumentMarginRateData[marginRate.data.InstrumentID] = marginRate.data;
        }
#ifdef _DEBUG
        std::cout << "[LocalCTP] Total instrument marinRate count from table in database: "
            << marinRateSqlValues.size() << std::endl;
#endif
        CSqliteHandler::SQL_VALUES commissionRateSqlValues;
        sqlHandler.SelectData(CThostFtdcInstrumentCommissionRateFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            commissionRateSqlValues);
        for (const auto& rowData : commissionRateSqlValues)
        {
            CThostFtdcInstrumentCommissionRateFieldWrapper commissionRate(rowData);
            m_instrumentCommissionRateData[commissionRate.data.InstrumentID] = commissionRate.data;
        }
#ifdef _DEBUG
        std::cout << "[LocalCTP] Total instrument CommissionRate count from table in database: "
            << commissionRateSqlValues.size() << std::endl;
#endif
    };
    reloadMarginAndCommissionRate();

    auto reloadPosition = [&]() {
        m_positionData.clear();
        CSqliteHandler::SQL_VALUES posSqlValues;
        sqlHandler.SelectData(
            CThostFtdcInvestorPositionFieldWrapper::generateSelectSqlByUserID(m_brokerID,m_userID),
            posSqlValues);
        CSqliteHandler::SQL_VALUES posDetailSqlValues;
        sqlHandler.SelectData(
            CThostFtdcInvestorPositionDetailFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            posDetailSqlValues);
        std::vector<CThostFtdcInvestorPositionDetailField> posDetails;
        posDetails.reserve(posDetailSqlValues.size());
        for (const auto& rowData : posDetailSqlValues)
        {
            posDetails.emplace_back(CThostFtdcInvestorPositionDetailFieldWrapper(rowData));
        }
        for (const auto& rowData : posSqlValues)
        {
            CThostFtdcInvestorPositionFieldWrapper wrapper(rowData);
            auto itInstr = m_instrData.find(wrapper.data.InstrumentID);
            if (itInstr == m_instrData.end())
            {
                continue;
            }
            PositionData posData;
            posData.pos = wrapper;
            posData.volumeMultiple = itInstr->second.VolumeMultiple;
            for (const auto& posDetail : posDetails)
            {
                auto posDetailMatchPos = [&]() -> bool {
                    bool isMatch = strcmp(posDetail.InstrumentID, posData.pos.InstrumentID) == 0 &&
                        strcmp(posDetail.ExchangeID, posData.pos.ExchangeID) == 0 &&
                        ((posDetail.Direction == THOST_FTDC_D_Buy && posData.pos.PosiDirection == THOST_FTDC_PD_Long) ||
                            (posDetail.Direction == THOST_FTDC_D_Sell && posData.pos.PosiDirection == THOST_FTDC_PD_Short));
                    if (!isMatch) return isMatch;
                    if (isSpecialExchange(posData.pos.ExchangeID))
                    {
                        if (posData.pos.PositionDate == THOST_FTDC_PSD_Today)
                            return strcmp(posDetail.OpenDate, posDetail.TradingDay) == 0;
                        else
                            return strcmp(posDetail.OpenDate, posDetail.TradingDay) != 0;
                    }
                    else
                    {
                        return true;
                    }
                };
                if (posDetailMatchPos())
                {
                    posData.posDetailData.emplace_back(posDetail);
                }
            }
            posData.sortPositionDetail();
            m_positionData.emplace(generatePositionKey(posData.pos), posData);
        }
    };
    reloadPosition();

    auto reloadTradingAccount = [&]() -> bool {
        CSqliteHandler::SQL_VALUES sqlValues;
        sqlHandler.SelectData(
            CThostFtdcTradingAccountFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            sqlValues);
        for (const auto& rowData : sqlValues)
        {
            CThostFtdcTradingAccountFieldWrapper wrapper(rowData);
            m_tradingAccount = wrapper;
            return true;
        }
        return false;
    };
    if (!reloadTradingAccount())
    {
        saveTradingAccountToDb(); // 如果此账户是初次创建,则数据库中还不存在其的记录,因此需保存到数据库中    
    }

    std::vector<CThostFtdcOrderField> ordersInDb;
    std::vector<CThostFtdcTradeFieldWrapper> tradesInDb;
    auto reloadOrder = [&]() {
        CSqliteHandler::SQL_VALUES orderSqlValues;
        sqlHandler.SelectData(
            CThostFtdcOrderFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            orderSqlValues);
        for (const auto& rowData : orderSqlValues)
        {
            CThostFtdcOrderFieldWrapper wrapper(rowData);
            if (TradingDay !=  wrapper.data.TradingDay)// only select today's orders
            {
                continue;
            }
            ordersInDb.emplace_back(wrapper.data);
        }
    };
    auto reloadTrade = [&]() {
        CSqliteHandler::SQL_VALUES tradeSqlValues;
        sqlHandler.SelectData(
            CThostFtdcTradeFieldWrapper::generateSelectSqlByUserID(m_brokerID, m_userID),
            tradeSqlValues);
        for (const auto& rowData : tradeSqlValues)
        {
            CThostFtdcTradeFieldWrapper wrapper(rowData);
            if (TradingDay != wrapper.data.TradingDay)// only select today's trades
            {
                continue;
            }
            tradesInDb.emplace_back(wrapper);
        }
    };
    reloadOrder();
    reloadTrade();

    auto generateOrderData = [&]() {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        for (auto& order : ordersInDb)
        {
            OrderData o(this, order);
            for (auto& trade : tradesInDb)
            {
                if (strcmp(trade.data.OrderSysID, order.OrderSysID) == 0 &&
                    strcmp(trade.data.ExchangeID, order.ExchangeID) == 0)
                {
                    o.rtnTrades.emplace_back(trade);
                }
            }
            const auto sessionKey = generateSessionKey(order.FrontID, order.SessionID);
            m_orderData[sessionKey].emplace(::atoi(order.OrderRef), o);
        }
    };
    generateOrderData();
}

void CLocalTraderApi::saveTradingAccountToDb()
{
    return saveDataToDb(CThostFtdcTradingAccountFieldWrapper(m_tradingAccount));
}

void CLocalTraderApi::savePositionToDb(const PositionData& pos)
{
    CSqliteTransactionHandler transactionHandle(sqlHandler);
    savePositionToDb(pos.pos);
    for (const auto& posDetail : pos.posDetailData)
    {
        savePositionDetialToDb(posDetail);
    }
}

void CLocalTraderApi::savePositionToDb(const CThostFtdcInvestorPositionField& pos)
{
    return saveDataToDb(CThostFtdcInvestorPositionFieldWrapper(pos));
}

void CLocalTraderApi::savePositionDetialToDb(const CThostFtdcInvestorPositionDetailField& posDetail)
{
    return saveDataToDb(CThostFtdcInvestorPositionDetailFieldWrapper(posDetail));
}

void CLocalTraderApi::saveOrderToDb(const CThostFtdcOrderField& order)
{
    return saveDataToDb(CThostFtdcOrderFieldWrapper(order));
}


///创建TraderApi
///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
///@return 创建出的UserApi
CThostFtdcTraderApi* CThostFtdcTraderApi::CreateFtdcTraderApi(const char *pszFlowPath/* = ""*/) {
    if (CLocalTraderApi::trade_api_set.empty())
    {
        CLocalTraderApi::initInstrMap();

        if (CLocalTraderApi::m_runningMode == RUNNING_MODE::BACKTEST_MODE)
        {
            //回测模式下会先删除数据库中所有交易数据.
            auto deleteAllAccountDataInDB = [&]() {
                const std::vector<std::string> toBeDeletedTables{
                    "CThostFtdcInvestorPositionField", "CThostFtdcInvestorPositionDetailField",
                    "CThostFtdcOrderField","CThostFtdcTradeField", "CThostFtdcTradingAccountField",
                    "CloseDetail", "SettlementData" };
                for (const auto& tableName : toBeDeletedTables)
                {
                    const std::string deleteTableSql = "DELETE FROM '" + tableName + "';";
                    CLocalTraderApi::sqlHandler.Delete(deleteTableSql);
                }
            };
            deleteAllAccountDataInDB();
        }
    }
	auto sp_this = std::make_shared<CLocalTraderApi>(pszFlowPath);
    CLocalTraderApi::trade_api_set.insert(sp_this);
	return sp_this.get();
}

///获取API的版本信息
///@retrun 获取到的版本号
const char* CThostFtdcTraderApi::GetApiVersion() { 

	return "LocalCTP V1.0.0 By QiuShui(Aura) QQ1005018695";
}

///删除接口对象本身
///@remark 不再使用本接口对象时,调用该函数删除接口对象
void CLocalTraderApi::Release() {
	auto sp_this = shared_from_this();
    CLocalTraderApi::trade_api_set.erase(sp_this);
}

void CLocalTraderApi::initInstrMap()
{
    static bool bFirstRun = true;
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    if (!bFirstRun)
    {
        return;
    }
    else
    {
        bFirstRun = false;
    }
    // create some tables in database
    CREATE_SQL_TABLE(CThostFtdcInvestorPositionField);
    CREATE_SQL_TABLE(CThostFtdcInvestorPositionDetailField);
    CREATE_SQL_TABLE(CThostFtdcOrderField);
    CREATE_SQL_TABLE(CThostFtdcTradeField);
    CREATE_SQL_TABLE(CThostFtdcTradingAccountField);
    CREATE_SQL_TABLE(CThostFtdcInstrumentField);
    CREATE_SQL_TABLE(CThostFtdcInstrumentMarginRateField);
    CREATE_SQL_TABLE(CThostFtdcInstrumentCommissionRateField);
    CREATE_SQL_TABLE(CloseDetail);//创建平仓表, 存储平仓明细记录
    CREATE_SQL_TABLE(SettlementData);//创建结算单表, 存储结算单

    auto readInstrumentFromCsvFile = [&]() -> bool
    {
        // 从当前目录(或环境变量中的目录)的 instrument.csv 以及数据库中的合约表中读取合约信息
        std::ifstream ifs("instrument.csv");
        if (!ifs.is_open())
        {
            return false;
        }
        std::string singleLine;
        if (!std::getline(ifs, singleLine))//第一行是表头
        {
            return false;
        }
        while (std::getline(ifs, singleLine))
        {
            if (singleLine.empty())
            {
                break;
            }
            std::istringstream iss(singleLine);
            CThostFtdcInstrumentField instr = { 0 };
            iss >> instr;
            m_instrData[instr.InstrumentID] = instr;
        }
#ifdef _DEBUG
        std::cout << "[LocalCTP] Total instrument count from instrument.csv: " << m_instrData.size() << std::endl;
#endif
        return true;
    };
    const bool readRet = readInstrumentFromCsvFile();
    const auto readFromCsvFile = m_instrData;

    CSqliteHandler::SQL_VALUES instrumentSqlValues;
    sqlHandler.SelectData(CThostFtdcInstrumentFieldWrapper::SELECT_SQL, instrumentSqlValues);
    for (const auto& rowData : instrumentSqlValues)
    {
        CThostFtdcInstrumentFieldWrapper instrument(rowData);
        m_instrData[instrument.data.InstrumentID] = instrument.data;
    }
#ifdef _DEBUG
    std::cout << "[LocalCTP] Total instrument count from instrument table in database: "
        << instrumentSqlValues.size() << std::endl;
#endif
    // 将从csv文件获取的合约数据,重新写入数据库中
    if (readRet)
    {
        CSqliteTransactionHandler transactionHandle(sqlHandler);
        for (const auto& instr : readFromCsvFile)
        {
            sqlHandler.Insert(
                CThostFtdcInstrumentFieldWrapper(instr.second).generateInsertSql());
        }
    }

    auto initProductsAndExchanges = [&]() {
        for (const auto& instrPair : m_instrData)
        {
            const auto& instr = instrPair.second;
            if (m_exchanges.find(instr.ExchangeID) == m_exchanges.end())
            {
                CThostFtdcExchangeField exchange = { 0 };
                exchange.ExchangeProperty = THOST_FTDC_EXP_Normal;
                strncpy(exchange.ExchangeID, instr.ExchangeID, sizeof(exchange.ExchangeID));
                strncpy(exchange.ExchangeName, exchange.ExchangeID, sizeof(exchange.ExchangeName));
                m_exchanges.emplace(exchange.ExchangeID, exchange);
            }

            if (m_products.find(instr.ProductID) == m_products.end())
            {
                auto getRemoveLastNumberStr = [&](std::string s) -> std::string {
                    //去除末尾的"月"字. example: 菜油1月 -> 菜油1
                    if (s.substr(s.size() - 2, 2) == STR_YUE)
                    {
                        s = s.substr(0, s.size() - 2);
                    }
                    //去除字符串末尾的数字. example: 黄金2402 -> 黄金
                    auto lastNotNumberIndex = s.find_last_not_of("0123456789");
                    if (lastNotNumberIndex == std::string::npos)
                    {
                        return s;
                    }
                    return s.substr(0, lastNotNumberIndex + 1);
                };
                auto getProductFronInstrument = [&]() -> CThostFtdcProductField {
                    CThostFtdcProductField p = { 0 };
                    ///产品名称
                    strncpy(p.ProductName, getRemoveLastNumberStr(instr.InstrumentName).c_str(),
                        sizeof(p.ProductName));
                    ///交易所代码
                    strncpy(p.ExchangeID, instr.ExchangeID, sizeof(p.ExchangeID));
                    ///产品类型
                    p.ProductClass = instr.ProductClass;
                    ///合约数量乘数
                    p.VolumeMultiple = instr.VolumeMultiple;
                    ///最小变动价位
                    p.PriceTick = instr.PriceTick;
                    ///市价单最大下单量
                    p.MaxMarketOrderVolume = instr.MaxMarketOrderVolume;
                    ///市价单最小下单量
                    p.MinMarketOrderVolume = instr.MinMarketOrderVolume;
                    ///限价单最大下单量
                    p.MaxLimitOrderVolume = instr.MaxLimitOrderVolume;
                    ///限价单最小下单量
                    p.MinLimitOrderVolume = instr.MinLimitOrderVolume;
                    ///持仓类型
                    p.PositionType = instr.PositionType;
                    ///持仓日期类型
                    p.PositionDateType = instr.PositionDateType;
                    ///平仓处理类型
                    p.CloseDealType = THOST_FTDC_CDT_Normal;
                    ///交易币种类型
                    strncpy(p.TradeCurrencyID, "CNY", sizeof(p.TradeCurrencyID));
                    ///质押资金可用范围
                    p.MortgageFundUseRange = THOST_FTDC_MFUR_None;
                    ///合约基础商品乘数
                    p.UnderlyingMultiple = instr.UnderlyingMultiple;
                    ///产品代码
                    strncpy(p.ProductID, instr.ProductID, sizeof(p.ProductID));
                    ///交易所产品代码
                    strncpy(p.ExchangeProductID, p.ProductID, sizeof(p.ExchangeProductID));
                    return p;
                };

                m_products.emplace(instr.ProductID, getProductFronInstrument());
            }
        }
    };
    initProductsAndExchanges();
}

///初始化
///@remark 初始化运行环境,只有调用后,接口才开始工作
void CLocalTraderApi::Init() {
    m_bRunning = true;

    // 从数据库中读取合约的保证金率和手续费.
    // 临时措施:对数据库中没有数据的合约,将合约的保证金率和手续费率初始化(保证金率为10%,手续费为1元每手)
    auto initializeCommissionRateAndMarginRate = [&]() {
        CThostFtdcInstrumentMarginRateField MarginRate = { 0 };
        MarginRate.LongMarginRatioByMoney = 0.1;
        MarginRate.ShortMarginRatioByMoney = 0.1;
        CThostFtdcInstrumentCommissionRateField CommissionRate = { 0 };
        CommissionRate.CloseRatioByVolume = 1;
        CommissionRate.CloseTodayRatioByVolume = 1;
        CommissionRate.OpenRatioByVolume = 1;

        for (const auto& instr : m_instrData)
        {
            strncpy(MarginRate.ExchangeID, instr.second.ExchangeID, sizeof(MarginRate.ExchangeID));
            strncpy(MarginRate.InstrumentID, instr.second.InstrumentID, sizeof(MarginRate.InstrumentID));
            m_instrumentMarginRateData[instr.first] = MarginRate;
            strncpy(CommissionRate.ExchangeID, instr.second.ExchangeID, sizeof(CommissionRate.ExchangeID));
            strncpy(CommissionRate.InstrumentID, instr.second.InstrumentID, sizeof(CommissionRate.InstrumentID));
            m_instrumentCommissionRateData[instr.first] = CommissionRate;
        }
    };
    initializeCommissionRateAndMarginRate();

    m_messageQueue.addMsg(OnFrontConnectedMsg());
    return;
}

///等待接口线程结束运行
///@return 线程退出代码
int CLocalTraderApi::Join() {
    // Do nothing
	return 0;
}

CLeeDateTime CLocalTraderApi::getNowTime()
{
    switch (m_runningMode)
    {
    case RUNNING_MODE::REALTIME_MODE:
        return CLeeDateTime::now();
    case RUNNING_MODE::BACKTEST_MODE:
        return (m_latestMarketTime == CLeeDateTime() ?
            m_defaultTimeInBackTestMode : m_latestMarketTime);
    case RUNNING_MODE::NONE:
    default:
        return CLeeDateTime::now();
    }
}

const char* CLocalTraderApi::StaticGetTradingDay() {
    static std::mutex tradingDayMutex;
    std::lock_guard<std::mutex> tradingDayGuard(tradingDayMutex);
    if (CLocalTraderApi::tradingDay.empty())
    {
        std::cout << "[LocalCTP] tradingDay is empty, let's init it!" << std::endl;
        auto getRawTradingDay = []() ->std::string
        {
            // use ( now time + 4 hours) as trading date,
            // it will consider the weekend, but not the holiday.
            // for example:
            // 1. 2023-08-07 10:00 -> 2023-08-07 14:00 -> return "20230807"
            // 2. 2023-08-07 20:00 -> 2023-08-08 02:00 -> return "20230808"
            // 3. 2023-08-04 20:00(Fri) -> 2023-08-05 02:00(Sat) -> 2023-08-07 02:00(Mon) -> return "20230807"
            auto checkTime = CLocalTraderApi::getNowTime() + CLeeDateTimeSpan(0, 4, 0, 0);
            if (isTradingDay(checkTime))
            {
                return checkTime.Format("%Y%m%d");
            }
            else
            {
                return getNextTradingDay(checkTime);
            }
        };
        const std::string rawTradingDay = getRawTradingDay();
        if (CLocalTraderApi::m_runningMode == RUNNING_MODE::BACKTEST_MODE)
        {
            //回测模式下初始化时不读取数据库结算单表来设置交易日,
            //也就是说即使结算单表中有当天日期的结算单, 仍然会以当天(而非下一天)作为交易日.
            //(不过现在回测模式下启动时会将数据库里所有账户数据清空,因此其实并不会发生上述情况)
            CLocalTraderApi::tradingDay = rawTradingDay;
            return CLocalTraderApi::tradingDay.c_str();;
        }
        CSqliteHandler::SQL_VALUES sqlValues;
        auto selectRet = CLocalTraderApi::sqlHandler.SelectData(
            "SELECT TradingDay FROM 'SettlementData' ORDER BY TradingDay DESC LIMIT 1;",
            sqlValues);
        if (!selectRet || sqlValues.empty())
        {
            CLocalTraderApi::tradingDay = rawTradingDay;
        }
        else
        {
            std::string tradingDayFromSettlementData = sqlValues.front().at("TradingDay");
            CLeeDateTime tradingDateFromSettlementData(
                std::stoi(tradingDayFromSettlementData.substr(0, 4)),
                std::stoi(tradingDayFromSettlementData.substr(4, 2)),
                std::stoi(tradingDayFromSettlementData.substr(6, 2)));
            tradingDayFromSettlementData = getNextTradingDay(tradingDateFromSettlementData);

            CLocalTraderApi::tradingDay = (std::max)(tradingDayFromSettlementData, rawTradingDay);
        }
    }
    return CLocalTraderApi::tradingDay.c_str();
}

///获取当前交易日
///@retrun 获取到的交易日
const char* CLocalTraderApi::GetTradingDay() {
    return CLocalTraderApi::StaticGetTradingDay();
}

///注册前置机网络地址
///@param pszFrontAddress：前置机网络地址。
///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。 
///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
// 本API不联网, 无需注册到前置机.
void CLocalTraderApi::RegisterFront(char *pszFrontAddress) { return; }

///注册名字服务器网络地址
///@param pszNsAddress：名字服务器网络地址。
///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:12001”。 
///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”12001”代表服务器端口号。
///@remark RegisterNameServer优先于RegisterFront
void CLocalTraderApi::RegisterNameServer(char *pszNsAddress) { return; }

///注册名字服务器用户信息
///@param pFensUserInfo：用户信息。
// 本接口被魔改, 接收的参数实际需要是一个行情快照的指针(CThostFtdcDepthMarketDataField*, 6.5.1及以后版本的).
// 使用者需要通过调用此接口, 来给API输入行情用于更新API内部的行情数据.
void CLocalTraderApi::RegisterFensUserInfo(CThostFtdcFensUserInfoField* pFensUserInfo) {
    if (pFensUserInfo == nullptr) return;
    CThostFtdcDepthMarketDataField* md = reinterpret_cast<CThostFtdcDepthMarketDataField*>(pFensUserInfo);

    onSnapshot(*md);
}

///注册回调接口
///@param pSpi 派生自回调接口类的实例
void CLocalTraderApi::RegisterSpi(CThostFtdcTraderSpi *pSpi) {
    m_messageQueue.RegisterSpi(pSpi);
    return;
}

///订阅私有流。
///@param nResumeType 私有流重传方式  
///        THOST_TERT_RESTART:从本交易日开始重传
///        THOST_TERT_RESUME:从上次收到的续传
///        THOST_TERT_QUICK:只传送登录后私有流的内容
///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
void CLocalTraderApi::SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType) { return; }

///订阅公共流。
///@param nResumeType 公共流重传方式  
///        THOST_TERT_RESTART:从本交易日开始重传
///        THOST_TERT_RESUME:从上次收到的续传
///        THOST_TERT_QUICK:只传送登录后公共流的内容
///        THOST_TERT_NONE:取消订阅公共流
///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
void CLocalTraderApi::SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType) { return; }

///客户端认证请求
int CLocalTraderApi::ReqAuthenticate(CThostFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID) {
    if (pReqAuthenticateField == nullptr || !m_bRunning) return -1;
    CThostFtdcRspAuthenticateField RspAuthenticateField = { 0 };
    memcpy(&RspAuthenticateField, pReqAuthenticateField, sizeof(CThostFtdcRspAuthenticateField));
    if (strlen(pReqAuthenticateField->UserID) == 0 ||
        strlen(pReqAuthenticateField->BrokerID) == 0)
    {
        m_messageQueue.addMsg(OnRspAuthenticateMsg(&RspAuthenticateField, setErrorMsgAndGetRspInfo(ErrMsgUserInfoIsEmpty), nRequestID, true));
        return 0;
    }
    if ((!m_userID.empty() && m_userID != pReqAuthenticateField->UserID) ||
        (!m_brokerID.empty() && m_brokerID != pReqAuthenticateField->BrokerID))
    {
        m_messageQueue.addMsg(OnRspAuthenticateMsg(&RspAuthenticateField, setErrorMsgAndGetRspInfo(ErrMsgUserInfoNotSameAsLastTime), nRequestID, true));
        return 0;
    }

    m_authenticated = true;
    m_userID = pReqAuthenticateField->UserID;
    m_brokerID = pReqAuthenticateField->BrokerID;
    strncpy(m_tradingAccount.AccountID, pReqAuthenticateField->UserID, sizeof(m_tradingAccount.AccountID));
    strncpy(m_tradingAccount.BrokerID, pReqAuthenticateField->BrokerID, sizeof(m_tradingAccount.BrokerID));
    m_messageQueue.addMsg(OnRspAuthenticateMsg(&RspAuthenticateField, &m_successRspInfo, nRequestID, true));
    return 0;
}

///用户登录请求
int CLocalTraderApi::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) {
    if (pReqUserLoginField == nullptr)
    {
        return -1;
    }
    CThostFtdcRspUserLoginField RspUserLogin = { 0 };
    strncpy(RspUserLogin.BrokerID, pReqUserLoginField->BrokerID, sizeof(RspUserLogin.BrokerID));
    strncpy(RspUserLogin.UserID, pReqUserLoginField->UserID, sizeof(RspUserLogin.UserID));
    if (!m_authenticated)
    {
        m_messageQueue.addMsg(OnRspUserLoginMsg(&RspUserLogin, setErrorMsgAndGetRspInfo(ErrMsgNotAuth), nRequestID, true));
        return 0;
    }
    if (m_userID != pReqUserLoginField->UserID || m_brokerID != pReqUserLoginField->BrokerID)
    {
        m_messageQueue.addMsg(OnRspUserLoginMsg(&RspUserLogin, setErrorMsgAndGetRspInfo(ErrMsgUserInfoNotSameAsAuth), nRequestID, true));
        return 0;
    }
    m_logined = true;
    //加载账户的数据
    reloadAccountData();

    strncpy(RspUserLogin.TradingDay, GetTradingDay(), sizeof(RspUserLogin.TradingDay));
    strncpy(RspUserLogin.LoginTime, CLocalTraderApi::getNowTime() //CLeeDateTime::GetCurrentTime()
        .Format("%H:%M:%S").c_str(),
        sizeof(RspUserLogin.LoginTime));
    strncpy(RspUserLogin.SHFETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.SHFETime));
    strncpy(RspUserLogin.DCETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.DCETime));
    strncpy(RspUserLogin.CZCETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.CZCETime));
    strncpy(RspUserLogin.FFEXTime, RspUserLogin.LoginTime, sizeof(RspUserLogin.FFEXTime));
    strncpy(RspUserLogin.INETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.INETime));
    strncpy(RspUserLogin.SystemName, "LocalCTP", sizeof(RspUserLogin.SystemName));
    strncpy(RspUserLogin.MaxOrderRef, "1", sizeof(RspUserLogin.MaxOrderRef));
    RspUserLogin.FrontID = m_frontID;
    m_sessionID = maxSessionID++;
    RspUserLogin.SessionID = m_sessionID;
    m_messageQueue.addMsg(OnRspUserLoginMsg(&RspUserLogin, &m_successRspInfo, nRequestID, true));
    return 0;
}

///登出请求
//登出后账户数据(含订单,持仓,资金等)仍然保留,仍然可以传入行情数据并更新资金等数据,因此实际不需要登出
int CLocalTraderApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID) {
    CHECK_LOGIN_USER(pUserLogout);

    m_authenticated = false;
    m_logined = false;
    CThostFtdcUserLogoutField RspUserLogout = { 0 };
    strncpy(RspUserLogout.UserID, pUserLogout->UserID, sizeof(RspUserLogout.UserID));
    strncpy(RspUserLogout.BrokerID, pUserLogout->BrokerID, sizeof(RspUserLogout.BrokerID));
    m_messageQueue.addMsg(OnRspUserLogoutMsg(&RspUserLogout, &m_successRspInfo, nRequestID, true));
#if 0
    //另一种方案:不允许登出. "上了车还想跑? 车门已焊死!"
    m_messageQueue.addMsg(OnRspUserLogoutMsg(&RspUserLogout, &setErrorMsgAndGetRspInfo("Logout is not supported in this system."),
        nRequestID, true));
#endif
    return 0;
}

///用户口令更新请求
int CLocalTraderApi::ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) {
    CHECK_LOGIN_USER(pUserPasswordUpdate);
    m_messageQueue.addMsg(OnRspUserPasswordUpdateMsg(nullptr,
        setErrorMsgAndGetRspInfo("Update password is not supported in this system."),
        nRequestID, true));
    return 0;
}

///资金账户口令更新请求
int CLocalTraderApi::ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) {
    CHECK_LOGIN_ACCOUNT(pTradingAccountPasswordUpdate);
    m_messageQueue.addMsg(OnRspTradingAccountPasswordUpdateMsg(nullptr,
        setErrorMsgAndGetRspInfo("Update password is not supported in this system."),
        nRequestID, true));
    return 0;
}

///报单录入请求
int CLocalTraderApi::ReqOrderInsert(CThostFtdcInputOrderField* pInputOrder, int nRequestID) {
    return ReqOrderInsertImpl(pInputOrder, nRequestID);
}
int CLocalTraderApi::ReqOrderInsertImpl(CThostFtdcInputOrderField * pInputOrder, int nRequestID,
    std::string relativeOrderSysID /*= std::string()*/) {
    CHECK_LOGIN_INVESTOR(pInputOrder);
    SHOW_TIME(StartOrder)
    const auto sendRejectOrder = [&](const char* errMsg) {
        m_messageQueue.addMsg(OnRspOrderInsertMsg(pInputOrder, setErrorMsgAndGetRspInfo(errMsg), nRequestID, true));
        m_messageQueue.addMsg(OnErrRtnOrderInsertMsg(pInputOrder, setErrorMsgAndGetRspInfo(errMsg)));
    };

    if (pInputOrder->VolumeTotalOriginal <= 0)
    {
        sendRejectOrder(ErrMsg_INVALID_ORDERSIZE);
        return 0;
    }
    if (strlen(pInputOrder->ExchangeID) == 0)
    {
        sendRejectOrder(ErrMsg_EXCHANGE_ID_IS_WRONG);
        return 0;
    }

    if (pInputOrder->CombHedgeFlag[0] != THOST_FTDC_HF_Speculation)
    {
        sendRejectOrder(ErrMsg_BAD_FIELD_ONLY_SPECULATION);
        return 0;
    }

    if (pInputOrder->ContingentCondition != THOST_FTDC_CC_Immediately &&
        !isConditionalType(pInputOrder->ContingentCondition))
    {
        sendRejectOrder(ErrMsg_NotSupportContingentCondition);
        return 0;
    }

    if (pInputOrder->TimeCondition != THOST_FTDC_TC_GFD &&
        pInputOrder->TimeCondition != THOST_FTDC_TC_IOC)
    {
        sendRejectOrder(ErrMsg_NotSupportTimeCondition);
        return 0;
    }

    if (pInputOrder->OrderPriceType != THOST_FTDC_OPT_LimitPrice)
    {
        sendRejectOrder(ErrMsg_PRICETYPE_NOTSUPPORT_BYEXCHANGE);
        return 0;
    }

    auto itInstr = m_instrData.find(pInputOrder->InstrumentID);
    if (itInstr == m_instrData.end() ||
        strcmp(itInstr->second.ExchangeID, pInputOrder->ExchangeID) != 0)
    {
        sendRejectOrder(ErrMsg_INSTRUMENT_NOT_FOUND);
        return 0;
    }

    double dblMulti = pInputOrder->LimitPrice / itInstr->second.PriceTick;
    int intMulti = static_cast<int>( round(dblMulti) );

    if (NEZ(dblMulti - intMulti))
    {
        sendRejectOrder((ErrMsg_BAD_PRICE_VALUE + std::to_string(itInstr->second.PriceTick)).c_str());
        return 0;
    }

    MarketDataMap::iterator itMd;
    if (itInstr->second.ProductClass != THOST_FTDC_PC_Combination)
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        itMd = m_mdData.find(pInputOrder->InstrumentID);
        if (itMd == m_mdData.end())
        {
            sendRejectOrder((std::string(ErrMsg_NoMarketData) + pInputOrder->InstrumentID).c_str());
            return 0;
        }
    }

    std::string instr = pInputOrder->InstrumentID;
    auto directionT = pInputOrder->Direction;
    const int orderNum = pInputOrder->VolumeTotalOriginal;
    double totalFrozenMargin = 0;
    double totalFrozenCash = 0;
    MarketDataVec mdVec;
    auto handleOpen = [&](bool preCheck) -> std::pair<bool, std::string>
    {
        // 如果是开仓报单,增加(各单腿)持仓中的冻结保证金
        auto posKey = generatePositionKey(instr,
            directionT,
            THOST_FTDC_PSD_Today);
        auto itMarginRate = m_instrumentMarginRateData.find(instr);
        if (itMarginRate == m_instrumentMarginRateData.end())
        {
            return std::make_pair(false, ErrMsg_INSTRUMENT_MARGINRATE_NOT_FOUND + instr);
        }
        MarketDataMap::iterator itDepthMarketData = itMd;
        //如果是组合合约,则再次查找单腿合约的行情; 如果不是组合合约,则无需再次查找行情了
        if (itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            std::lock_guard<std::mutex> mdGuard(m_mdMtx);
            itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return std::make_pair(false, ErrMsg_NoMarketData + instr);
            }
        }
        if (preCheck)
        {
            mdVec.emplace_back(itDepthMarketData->second);
        }

        // 冻结保证金和保证金计算使用的价格,不同期货公司不一样.
        // 可以参见TThostFtdcMarginPriceTypeType类型取值说明.
        // 可通过查询经纪公司交易参数获得
        // (请求查询函数ReqQryBrokerTradingParams, 响应函数OnRspQryBrokerTradingParams)
        // 本系统以昨结算价作为计算冻结保证金的基准价格,
        // 而以昨结算价(对昨仓)和开仓成交价格(对今仓)作为计算持仓保证金时的基准价格
        double MarginRatioByMoney = (directionT == THOST_FTDC_D_Buy ?
            itMarginRate->second.LongMarginRatioByMoney
            : itMarginRate->second.ShortMarginRatioByMoney);
        double MarginRatioByVolume = (directionT == THOST_FTDC_D_Buy ?
            itMarginRate->second.LongMarginRatioByVolume
            : itMarginRate->second.ShortMarginRatioByVolume);
        double frozenCash(0);
        if (isOptions(itInstr->second.ProductClass))
        {
            if (directionT == THOST_FTDC_D_Buy)//期权买入的委托, 需冻结一部分权利金, 而无需支付保证金, 因此将其保证金率设为0
            {
                frozenCash = itDepthMarketData->second.PreSettlementPrice *
                    itInstr->second.VolumeMultiple * orderNum;
                MarginRatioByMoney = 0;
                MarginRatioByVolume = 0;
            }
            else//期权卖出的委托, 需冻结一部分保证金(期权保证金计算太复杂啦,本系统以期货保证金计算规则来算期权保证金)
            {
            }
        }
        const double frozenMargin = itDepthMarketData->second.PreSettlementPrice *
            itInstr->second.VolumeMultiple * orderNum * MarginRatioByMoney
            + orderNum * MarginRatioByVolume;

        totalFrozenMargin += frozenMargin;
        totalFrozenCash += frozenCash;

        PositionDataMap::iterator itPos;
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                PositionData tempPos;
                tempPos.volumeMultiple = itInstr->second.VolumeMultiple;
                strncpy(tempPos.pos.BrokerID, m_brokerID.c_str(), sizeof(tempPos.pos.BrokerID));
                strncpy(tempPos.pos.InvestorID, m_userID.c_str(), sizeof(tempPos.pos.InvestorID));
                tempPos.pos.HedgeFlag = pInputOrder->CombHedgeFlag[0];
                strncpy(tempPos.pos.ExchangeID, pInputOrder->ExchangeID, sizeof(tempPos.pos.ExchangeID));// 交易所代码
                strncpy(tempPos.pos.InstrumentID, instr.c_str(), sizeof(tempPos.pos.InstrumentID));// 合约代码
                tempPos.pos.PreSettlementPrice = itDepthMarketData->second.PreSettlementPrice;
                tempPos.pos.SettlementPrice = itDepthMarketData->second.SettlementPrice;
                strncpy(tempPos.pos.TradingDay, GetTradingDay(), sizeof(tempPos.pos.TradingDay));
                if (!preCheck)
                {
                    tempPos.pos.FrozenMargin = frozenMargin;// (因为开仓未成交而)冻结的保证金
                    tempPos.pos.FrozenCash = frozenCash;// (因为开仓未成交而)冻结的权利金
                }
                tempPos.pos.PosiDirection = getPositionDirectionFromDirection(directionT);// 持仓方向
                tempPos.pos.PositionDate = THOST_FTDC_PSD_Today;// 持仓日期类型(今仓)
                tempPos.pos.MarginRateByMoney = MarginRatioByMoney;
                tempPos.pos.MarginRateByVolume = MarginRatioByVolume;
                std::tie(itPos, std::ignore) = m_positionData.emplace(posKey, tempPos);
            }
            else
            {
                if (!preCheck)
                {
                    itPos->second.pos.FrozenMargin += frozenMargin;
                    itPos->second.pos.FrozenCash += frozenCash;
                }
            }
        }
        if (preCheck)
        {
            double newAvailable = m_tradingAccount.Balance - m_tradingAccount.CurrMargin
                - m_tradingAccount.FrozenMargin
                - m_tradingAccount.FrozenCash
                - totalFrozenMargin
                - totalFrozenCash;
            if (LTZ(newAvailable))
            {
                return std::make_pair(false, ERRMSG_AVAILABLE_NOT_ENOUGH);;
            }
        }
        else
        {
            CSqliteTransactionHandler transactionHandle(sqlHandler);
            savePositionToDb(itPos->second.pos);
            m_tradingAccount.FrozenMargin += frozenMargin;
            m_tradingAccount.FrozenCash += frozenCash;
            if (NEZ(frozenMargin) || NEZ(frozenCash))
            {
                updatePNL();
            }
        }
        return std::make_pair(true, std::string());
    };
    auto handleClose = [&](bool preCheck) -> std::pair<bool, std::string> {
        // 如果是平仓报单,则校验(各单腿)可平仓数量和增加(各单腿)冻结持仓数量
        auto posKey = generatePositionKey(instr,
            directionT,
            getDateTypeFromOffset(itInstr->second.ExchangeID, pInputOrder->CombOffsetFlag[0]));

        MarketDataMap::iterator itDepthMarketData = itMd;
        //如果是组合合约,则再次查找单腿合约的行情; 如果不是组合合约,则无需再次查找行情了
        if (itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            std::lock_guard<std::mutex> mdGuard(m_mdMtx);
            itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return std::make_pair(false, ErrMsg_NoMarketData + instr);
            }
        }
        if (preCheck)
        {
            mdVec.emplace_back(itDepthMarketData->second);
        }

        double frozenCash(0);
        if (isOptions(itInstr->second.ProductClass))
        {
            if (getOppositeDirection(directionT) == THOST_FTDC_D_Buy)//期权买入的委托, 需冻结一部分权利金
            {
                frozenCash = itDepthMarketData->second.PreSettlementPrice *
                    itInstr->second.VolumeMultiple * orderNum;
            }
            else//期权卖出的委托
            {
            }
        }
        totalFrozenCash += frozenCash;

        PositionDataMap::iterator itPos;
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                return std::make_pair(false, ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH + std::string("0")
                    + " on " + instr);
            }
        }
        int& frozenPositionNum = (itPos->second.pos.PosiDirection == THOST_FTDC_PD_Long ?
            itPos->second.pos.ShortFrozen : itPos->second.pos.LongFrozen);
        const auto closable = itPos->second.pos.Position - frozenPositionNum;
        if (closable < orderNum)
        {
            return std::make_pair(false,
                ((isSpecialExchange(itInstr->second.ExchangeID) && pInputOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) ?
                    ERRMSG_AVAILABLE_TODAY_POSITION_NOT_ENOUGH : ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH)
                + std::to_string(closable) + " on " + instr);
        }
        if (!preCheck)
        {
            frozenPositionNum += orderNum;
            itPos->second.pos.FrozenCash += frozenCash;
            CSqliteTransactionHandler transactionHandle(sqlHandler);
            savePositionToDb(itPos->second.pos);
            m_tradingAccount.FrozenCash += frozenCash;
            if (NEZ(frozenCash))
            {
                updatePNL();
            }
        }
        return std::make_pair(true, std::string());
    };

    auto doRiskCheck = [&](bool preCheck) -> std::pair<bool, std::string>
    {
        // not check riskcontrol on conditional orders
        if (isConditionalType(pInputOrder->ContingentCondition))
        {
            return std::make_pair(true, std::string());
        }
        if (isOpen(pInputOrder->CombOffsetFlag[0]))
        {
            if (itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
            {
                std::vector<std::string> singleContracts;
                GetSingleContractFromCombinationContract(itInstr->first, singleContracts);

                for (std::size_t legNo = 0; legNo < singleContracts.size(); ++legNo)
                {
                    instr = singleContracts[legNo];
                    directionT = (legNo % 2 == 0 ? pInputOrder->Direction : getOppositeDirection(pInputOrder->Direction));

                    auto handleSingleContractRet = ((pInputOrder->IsSwapOrder != 0 && legNo % 2 != 0) ?
                        handleClose(preCheck) : handleOpen(preCheck));//需考虑互换单的情况
                    if (!handleSingleContractRet.first)
                    {
                        return handleSingleContractRet;;
                    }
                }
            }
            else
            {
                directionT = pInputOrder->Direction;
                auto handleOpenRet = handleOpen(preCheck);
                if (!handleOpenRet.first)
                {
                    return handleOpenRet;
                }
            }
        }
        else
        {
            if (itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
            {
                std::vector<std::string> singleContracts;
                GetSingleContractFromCombinationContract(itInstr->first, singleContracts);

                for (std::size_t legNo = 0; legNo < singleContracts.size(); ++legNo)
                {
                    instr = singleContracts[legNo];
                    directionT = (legNo % 2 == 0 ? getOppositeDirection(pInputOrder->Direction) : pInputOrder->Direction);

                    auto handleSingleContractRet = ((pInputOrder->IsSwapOrder != 0 && legNo % 2 != 0) ?
                        handleOpen(preCheck) : handleClose(preCheck));//需考虑互换单的情况
                    if (!handleSingleContractRet.first)
                    {
                        return handleSingleContractRet;
                    }
                }
            }
            else
            {
                directionT = getOppositeDirection(pInputOrder->Direction); // 反方向的(准备平仓的)持仓方向
                auto handleCloseRet = handleClose(preCheck);
                if (!handleCloseRet.first)
                {
                    return handleCloseRet;
                }
            }
        }
        return std::make_pair(true, std::string());

    };

    auto checkRet = doRiskCheck(true);//风控预先校验
    if (!checkRet.first)
    {
        sendRejectOrder(checkRet.second.c_str());
        return 0;
    }
    SHOW_TIME(AfterDoRiskCheck1)
    std::map<int, OrderData>::iterator itOrder;
    {
        int OrderRef = ::atoi(pInputOrder->OrderRef);
        const auto sessionKey = generateSessionKey(m_frontID, m_sessionID);
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        auto getMaxOrderRef = [&](int& _maxOrderRef) -> bool
        {
            auto itSessionOrder = m_orderData.find(sessionKey);
            if (itSessionOrder == m_orderData.end() || itSessionOrder->second.empty())
            {
                return false;
            }
            _maxOrderRef = (--(itSessionOrder->second.end()))->first;
            return true;
        };
        int maxOrderRef = 0;
        bool getmaxOrderRefRet = getMaxOrderRef(maxOrderRef);
        std::unique_ptr<OrderData> x;
        if (strlen(pInputOrder->OrderRef) != 0)// check the OrderRef if OrderRef in order is not empty
        {
            if(getmaxOrderRefRet && OrderRef < maxOrderRef)
            {
                sendRejectOrder(ErrMsgDuplicateOrder);
                return 0;
            }
            else
            {
                // OrderRef is allowed
                x.reset(new OrderData(this, *pInputOrder, relativeOrderSysID));
            }
        }
        else
        {
            // if OrderRef is empty, we need set it to a valid value
            OrderRef = getmaxOrderRefRet ? (maxOrderRef + 1) : 1;
            auto InputOrder = *pInputOrder;
            strncpy(InputOrder.OrderRef, std::to_string(OrderRef).c_str(),
                sizeof(InputOrder.OrderRef));
            x.reset(new OrderData(this, InputOrder, relativeOrderSysID));
        }

        bool emplaceSuccess(false);
        std::tie(itOrder, emplaceSuccess) = m_orderData[sessionKey].emplace(OrderRef, *x);
        if (!emplaceSuccess)
        {
            sendRejectOrder(ErrMsgDuplicateOrder);
            return 0;
        }

        if (isConditionalType(pInputOrder->ContingentCondition))
        {
            m_contionalOrders.emplace_back(&(itOrder->second));
            return 0;
        }
    }

    checkRet = doRiskCheck(false);//更新风控值
    SHOW_TIME(AfterDoRiskCheck2)
    /*if (!checkRet.first) //此前已经校验过,不会再为false
    {
        sendRejectOrder(checkRet.second.c_str());
        return 0;
    }*/
    {
        TradePriceVec priceVec;
        if(isMatchTrade(pInputOrder->Direction, pInputOrder->LimitPrice, mdVec, priceVec))
        {
            itOrder->second.handleTrade(priceVec, orderNum);
            return 0;
        }
        else
        {
            // 判断是否是IOC订单
            // 可能有的交易所不是这样判断,但本系统以这种方式来统一判断处理
            auto isIOCOrder = [&]() {
                return pInputOrder->TimeCondition == THOST_FTDC_TC_IOC &&
                    pInputOrder->VolumeCondition == THOST_FTDC_VC_CV;
            };
            if (isIOCOrder())
            {
                itOrder->second.handleCancel(false);
                return 0;
            }
        }
    }

    SHOW_TIME(ReqOrderDone)
    return 0;
}

///预埋单录入请求
int CLocalTraderApi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pParkedOrder);
    m_messageQueue.addMsg(OnRspParkedOrderInsertMsg(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true));
    return 0;
}

///预埋撤单录入请求
int CLocalTraderApi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pParkedOrderAction);
    m_messageQueue.addMsg(OnRspParkedOrderActionMsg(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true));
    return 0;
}

///报单操作请求
int CLocalTraderApi::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pInputOrderAction);

    if (pInputOrderAction->ActionFlag != THOST_FTDC_AF_Delete)
    {
        m_messageQueue.addMsg(OnRspOrderActionMsg(pInputOrderAction, setErrorMsgAndGetRspInfo(ErrMsg_NotSupportModifyOrder), nRequestID, true));
        return 0;
    }
    const auto sessionKey = generateSessionKey(pInputOrderAction->FrontID, pInputOrderAction->SessionID);
    int OrderRef = atoi(pInputOrderAction->OrderRef);
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        // first, we find order by "OrderRef + FrontID + SessionID" (and, the InstrumentID must be valid)
        auto it = m_orderData.find(sessionKey);
        if (it != m_orderData.end())
        {
            auto itOrder = it->second.find(OrderRef);
            if (itOrder != it->second.end())
            {
                auto& order = itOrder->second;
                if (order.isDone() ||
                    order.rtnOrder.FrontID != pInputOrderAction->FrontID ||
                    order.rtnOrder.SessionID != pInputOrderAction->SessionID ||
                    strcmp(order.rtnOrder.InstrumentID, pInputOrderAction->InstrumentID) != 0)
                {
                    m_messageQueue.addMsg(OnRspOrderActionMsg(pInputOrderAction, setErrorMsgAndGetRspInfo(
                        order.isDone() ? ErrMsg_AlreadyDoneOrder : ErrMsg_NotExistOrder),
                        nRequestID, true));
                    return 0;
                }
                else
                {
                    order.handleCancel();
                    return 0;
                }
            }
        }
        // second, we find order by "OrderSysID + ExchangeID"
        for (auto& sessionOrderPair : m_orderData)
        {
            for (auto& orderPair : sessionOrderPair.second)
            {
                auto& order = orderPair.second;
                if (strcmp(order.rtnOrder.OrderSysID, pInputOrderAction->OrderSysID) == 0 &&
                    strcmp(order.rtnOrder.ExchangeID, pInputOrderAction->ExchangeID) == 0)
                {
                    order.handleCancel();
                    return 0;
                }
            }
        }
    }
    m_messageQueue.addMsg(OnRspOrderActionMsg(pInputOrderAction, setErrorMsgAndGetRspInfo(ErrMsg_NotExistOrder), nRequestID, true));
    return 0;
}

/////查询最大报单数量请求
//int CLocalTraderApi::ReqQryMaxOrderVolume(CThostFtdcQryMaxOrderVolumeField *pQryMaxOrderVolume, int nRequestID) {
//    CHECK_LOGIN_INVESTOR(pQryMaxOrderVolume);
//    m_messageQueue.addMsg(OnRspQryMaxOrderVolumeMsg(nullptr,
//        setErrorMsgAndGetRspInfo("Query MaxOrderVolume is not supported in this system."),
//        nRequestID, true));
//    return 0;
//}

///投资者结算结果确认
int CLocalTraderApi::ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pSettlementInfoConfirm);
    const auto nowTime = CLocalTraderApi::getNowTime(); //CLeeDateTime::GetCurrentTime();
    //更新最新的交易日的结算结果确认信息
    const std::string UPDATE_NEWEST_SETTLEMENT_RECORD_TO_CONFIRMED =
        std::string("UPDATE 'SettlementData' SET ConfirmDay='") + GetTradingDay()
        + "', ConfirmTime='" + nowTime.Format("%H:%M:%S")
        + "' where BrokerID='" + m_brokerID
        + "' and InvestorID='" + m_userID
        + "' and ConfirmDay='' and TradingDay IN (SELECT MAX(TradingDay) FROM 'SettlementData');";
    CSqliteHandler::SQL_VALUES posSqlValues;
    sqlHandler.Update(UPDATE_NEWEST_SETTLEMENT_RECORD_TO_CONFIRMED);

    CThostFtdcSettlementInfoConfirmField SettlementInfoConfirm = *pSettlementInfoConfirm;
    m_messageQueue.addMsg(OnRspSettlementInfoConfirmMsg(&SettlementInfoConfirm, &m_successRspInfo, nRequestID, true));
    return 0;
}

///请求删除预埋单
int CLocalTraderApi::ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pRemoveParkedOrder);
    m_messageQueue.addMsg(OnRspRemoveParkedOrderMsg(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true));
    return 0;
}

///请求删除预埋撤单
int CLocalTraderApi::ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pRemoveParkedOrderAction);
    m_messageQueue.addMsg(OnRspRemoveParkedOrderActionMsg(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true));
    return 0;
}

///请求查询报单
int CLocalTraderApi::ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryOrder);
    std::vector<CThostFtdcOrderField*> v;
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        for (auto& sessionOrders : m_orderData)
        {
            for (auto& o : sessionOrders.second)
            {
                auto& rtnOrder = o.second.rtnOrder;
                if (COMPARE_MEMBER_MATCH(pQryOrder, rtnOrder, ExchangeID) &&
                    COMPARE_MEMBER_MATCH(pQryOrder, rtnOrder, OrderSysID) &&
                    COMPARE_MEMBER_MATCH(pQryOrder, rtnOrder, InstrumentID))
                {
                    v.emplace_back(&rtnOrder);
                }
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryOrderMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryOrderMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询成交
int CLocalTraderApi::ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryTrade);
    std::vector<CThostFtdcTradeField*> v;
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        for (auto& sessionOrders : m_orderData)
        {
            for (auto& o : sessionOrders.second)
            {
                for (auto& t : o.second.rtnTrades)
                {
                    if (COMPARE_MEMBER_MATCH(pQryTrade, t.data, ExchangeID) &&
                        COMPARE_MEMBER_MATCH(pQryTrade, t.data, TradeID) &&
                        COMPARE_MEMBER_MATCH(pQryTrade, t.data, InstrumentID))
                    {
                        v.emplace_back(&(t.data));
                    }
                }
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryTradeMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryTradeMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询投资者持仓
int CLocalTraderApi::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestorPosition);
    std::vector<CThostFtdcInvestorPositionField*> v;
    {
        std::lock_guard<std::mutex> posGuard(m_positionMtx);
        for (auto& o : m_positionData)
        {
            auto& pos = o.second.pos;
            if (COMPARE_MEMBER_MATCH(pQryInvestorPosition, pos, ExchangeID) &&
                COMPARE_MEMBER_MATCH(pQryInvestorPosition, pos, InstrumentID))
            {
                v.emplace_back(&pos);
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryInvestorPositionMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryInvestorPositionMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询资金账户
int CLocalTraderApi::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryTradingAccount);
    strncpy(m_tradingAccount.BrokerID, m_brokerID.c_str(), sizeof(m_tradingAccount.BrokerID));
    strncpy(m_tradingAccount.AccountID, m_userID.c_str(), sizeof(m_tradingAccount.AccountID));
    strncpy(m_tradingAccount.TradingDay, GetTradingDay(), sizeof(m_tradingAccount.TradingDay));
    m_messageQueue.addMsg(OnRspQryTradingAccountMsg(&m_tradingAccount, &m_successRspInfo, nRequestID, true));
    return 0;
}

///请求查询投资者
int CLocalTraderApi::ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestor);
    CThostFtdcInvestorField Investor = { 0 };
    strncpy(Investor.InvestorID, pQryInvestor->InvestorID, sizeof(Investor.InvestorID));
    strncpy(Investor.BrokerID, pQryInvestor->BrokerID, sizeof(Investor.BrokerID));
    Investor.IdentifiedCardType = THOST_FTDC_ICT_OtherCard;
    strncpy(Investor.IdentifiedCardNo, "QQ1005018695", sizeof(Investor.IdentifiedCardNo));
    Investor.IsActive = 1;
    m_messageQueue.addMsg(OnRspQryInvestorMsg(&Investor, &m_successRspInfo, nRequestID, true));
    return 0;
}

///请求查询合约保证金率
int CLocalTraderApi::ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID) {
    if (pQryInstrumentMarginRate == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcInstrumentMarginRateField*> v;
    for (auto& instrPair : m_instrumentMarginRateData)
    {
        auto& marginRate = instrPair.second;
        if (COMPARE_MEMBER_MATCH(pQryInstrumentMarginRate, marginRate, ExchangeID) &&
            COMPARE_MEMBER_MATCH(pQryInstrumentMarginRate, marginRate, InstrumentID))            
        {
            v.emplace_back(&marginRate);
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryInstrumentMarginRateMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryInstrumentMarginRateMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询合约手续费率
int CLocalTraderApi::ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) {
    if (pQryInstrumentCommissionRate == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcInstrumentCommissionRateField*> v;
    for (auto& instrPair : m_instrumentCommissionRateData)
    {
        auto& commissionRate = instrPair.second;
        if (COMPARE_MEMBER_MATCH(pQryInstrumentCommissionRate, commissionRate, ExchangeID) &&
            COMPARE_MEMBER_MATCH(pQryInstrumentCommissionRate, commissionRate, InstrumentID))
        {
            v.emplace_back(&commissionRate);
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryInstrumentCommissionRateMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryInstrumentCommissionRateMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询交易所
int CLocalTraderApi::ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID) {
    if (pQryExchange == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcExchangeField*> v;
    for (auto& e : m_exchanges)
    {
        if (COMPARE_MEMBER_MATCH(pQryExchange, e.second, ExchangeID))
        {
            v.emplace_back(&(e.second));
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryExchangeMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryExchangeMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询产品
int CLocalTraderApi::ReqQryProduct(CThostFtdcQryProductField *pQryProduct, int nRequestID) {
    if (pQryProduct == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcProductField*> v;
    for (auto& productPair : m_products)
    {
        auto& product = productPair.second;
        if (COMPARE_MEMBER_MATCH(pQryProduct, product, ExchangeID) &&
            COMPARE_MEMBER_MATCH(pQryProduct, product, ProductID) &&
            pQryProduct->ProductClass == product.ProductClass)
        {
            v.emplace_back(&product);
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryProductMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryProductMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询合约
int CLocalTraderApi::ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID) {
    if (pQryInstrument == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcInstrumentField*> v;
    v.reserve(1000); // maybe less than this count ?
    for (auto& instrPair : m_instrData)
    {
        CThostFtdcInstrumentField& instr = instrPair.second;
        if (COMPARE_MEMBER_MATCH(pQryInstrument, instr, ExchangeID) &&
            COMPARE_MEMBER_MATCH(pQryInstrument, instr, ProductID) &&
            COMPARE_MEMBER_MATCH(pQryInstrument, instr, InstrumentID))
        {
            v.emplace_back(&instr);   
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryInstrumentMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryInstrumentMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询行情
int CLocalTraderApi::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) {
    if (pQryDepthMarketData == nullptr || !m_logined) return -1;
    std::vector<CThostFtdcDepthMarketDataField*> v;
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        for (auto& p : m_mdData)
        {
            auto& md = p.second;
            if (COMPARE_MEMBER_MATCH(pQryDepthMarketData, md, ExchangeID) &&
                COMPARE_MEMBER_MATCH(pQryDepthMarketData, md, InstrumentID))
            {
                v.emplace_back(&md);
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryDepthMarketDataMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryDepthMarketDataMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询投资者结算结果
int CLocalTraderApi::ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQrySettlementInfo);

    const std::string TradingDayForQuery = pQrySettlementInfo->TradingDay;
    const std::string SELECT_NEWEST_SETTLEMENT_RECORD =
        "SELECT * FROM 'SettlementData' where BrokerID='" + m_brokerID
        + "' and InvestorID='" + m_userID
        + (!TradingDayForQuery.empty() ?
            ("' and TradingDay='" + TradingDayForQuery) : "")
        + "' ORDER BY TradingDay DESC LIMIT 1;";
    CSqliteHandler::SQL_VALUES posSqlValues;
    sqlHandler.SelectData(SELECT_NEWEST_SETTLEMENT_RECORD, posSqlValues);
    for (auto it = posSqlValues.begin(); it != posSqlValues.end(); ++it)
    {
        if (it->empty())
        {
            m_messageQueue.addMsg(OnRspQrySettlementInfoMsg(nullptr, &m_successRspInfo, nRequestID,
                (it + 1 == posSqlValues.end() ? true : false)));
        }
        else
        {
            const std::string SettlementContent = base64_decode(it->at("SettlementContent"));
            for (size_t now_index = 0; now_index < SettlementContent.size();)//遍历结算单正文, 持续推送直到完成.
            {
                bool bIsLast = false;//是否为最后一条记录

                CThostFtdcSettlementInfoField SettlementInfo = { 0 };
                strncpy(SettlementInfo.TradingDay, it->at("TradingDay").c_str(), sizeof(SettlementInfo.TradingDay));
                strncpy(SettlementInfo.BrokerID, pQrySettlementInfo->BrokerID, sizeof(SettlementInfo.BrokerID));
                strncpy(SettlementInfo.InvestorID, pQrySettlementInfo->InvestorID, sizeof(SettlementInfo.InvestorID));
                strncpy(SettlementInfo.AccountID, pQrySettlementInfo->InvestorID, sizeof(SettlementInfo.AccountID));
                strncpy(SettlementInfo.CurrencyID, "CNY", sizeof(SettlementInfo.CurrencyID));
                if (now_index + (sizeof(SettlementInfo.Content) - 1) < SettlementContent.size())
                {
                    strncpy(SettlementInfo.Content,
                        SettlementContent.substr(now_index, sizeof(SettlementInfo.Content) - 1).c_str(),
                        sizeof(SettlementInfo.Content) - 1);
                    now_index += sizeof(SettlementInfo.Content) - 1;
                }
                else
                {
                    strncpy(SettlementInfo.Content,
                        SettlementContent.substr(now_index, SettlementContent.size() - now_index).c_str(),
                        SettlementContent.size() - now_index);
                    now_index += SettlementContent.size() - now_index;
                    bIsLast = true;
                }
                m_messageQueue.addMsg(OnRspQrySettlementInfoMsg(&SettlementInfo, &m_successRspInfo, nRequestID, bIsLast));
            }
            break;
        }
    }
    if (posSqlValues.empty())
    {
        m_messageQueue.addMsg(OnRspQrySettlementInfoMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询投资者持仓明细
int CLocalTraderApi::ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestorPositionDetail);
    std::vector<CThostFtdcInvestorPositionDetailField*> v;
    {
        std::lock_guard<std::mutex> posGuard(m_positionMtx);
        for (auto& o : m_positionData)
        {
            for (auto& t : o.second.posDetailData)
            {
                if (COMPARE_MEMBER_MATCH(pQryInvestorPositionDetail, t, ExchangeID) &&
                    COMPARE_MEMBER_MATCH(pQryInvestorPositionDetail, t, InstrumentID))
                {
                    v.emplace_back(&t);
                }
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_messageQueue.addMsg(OnRspQryInvestorPositionDetailMsg(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false)));
    }
    if (v.empty())
    {
        m_messageQueue.addMsg(OnRspQryInvestorPositionDetailMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    return 0;
}

///请求查询结算信息确认
int CLocalTraderApi::ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQrySettlementInfoConfirm);
    const std::string SELECT_NEWEST_SETTLEMENT_RECORD =
        "SELECT ConfirmDay,ConfirmTime FROM 'SettlementData' where BrokerID='" + m_brokerID
        + "' and InvestorID='" + m_userID + "' ORDER BY TradingDay DESC LIMIT 1;";
    CSqliteHandler::SQL_VALUES posSqlValues;
    sqlHandler.SelectData(SELECT_NEWEST_SETTLEMENT_RECORD, posSqlValues);
    for (auto it = posSqlValues.begin(); it != posSqlValues.end(); ++it)
    {
        if (it->empty() || it->at("ConfirmDay").empty())
        {
            m_messageQueue.addMsg(OnRspQrySettlementInfoConfirmMsg(nullptr, &m_successRspInfo, nRequestID,
                (it + 1 == posSqlValues.end() ? true : false)));
        }
        else
        {
            CThostFtdcSettlementInfoConfirmField SettlementInfoConfirm = { 0 };
            strncpy(SettlementInfoConfirm.BrokerID, pQrySettlementInfoConfirm->BrokerID, sizeof(SettlementInfoConfirm.BrokerID));
            strncpy(SettlementInfoConfirm.InvestorID, pQrySettlementInfoConfirm->InvestorID, sizeof(SettlementInfoConfirm.InvestorID));
            strncpy(SettlementInfoConfirm.ConfirmDate, it->at("ConfirmDay").c_str(), sizeof(SettlementInfoConfirm.ConfirmDate));
            strncpy(SettlementInfoConfirm.ConfirmTime, it->at("ConfirmTime").c_str(), sizeof(SettlementInfoConfirm.ConfirmTime));
            strncpy(SettlementInfoConfirm.AccountID, pQrySettlementInfoConfirm->InvestorID, sizeof(SettlementInfoConfirm.AccountID));
            strncpy(SettlementInfoConfirm.CurrencyID, "CNY", sizeof(SettlementInfoConfirm.CurrencyID));
            m_messageQueue.addMsg(OnRspQrySettlementInfoConfirmMsg(&SettlementInfoConfirm, &m_successRspInfo, nRequestID,
                (it + 1 == posSqlValues.end() ? true : false)));
        }
    }
    if (posSqlValues.empty())
    {
        m_messageQueue.addMsg(OnRspQrySettlementInfoConfirmMsg(nullptr, &m_successRspInfo, nRequestID, true));
    }
    
    return 0;
}

///报价录入请求
// 本接口被改造为接收行情快照数据的接口.
// 具体字段对应:
// (CThostFtdcDepthMarketDataField 中的字段 -> CThostFtdcInputQuoteField pInputQuote 中的字段):
//
// 交易日: TradingDay -> BrokerID
// 业务日期: ActionDay -> InvestorID
// 交易所代码: ExchangeID -> ExchangeID(名字不变)
// 合约代码: InstrumentID -> InstrumentID(名字不变)
// 最后修改时间: UpdateTime -> ClientID
// 最后修改毫秒: UpdateMillisec -> RequestID(不是函数参数 nRequestID 哦)
// 数量(今日的成交量): Volume -> 函数参数 nRequestID(不是 RequestID 哦)
// 申买价一: BidPrice1 -> BidPrice
// 申卖价一: AskPrice1 -> AskPrice
// 申买量一: BidVolume1 -> BidVolume
// 申卖量一: AskVolume1 -> AskVolume
//
// 最新价: LastPrice -> UserID(字符串类型) 请将它转换为字符串.
// 涨停价: UpperLimitPrice -> BidOrderRef(字符串类型) 请将它转换为字符串.
// 跌停价: LowerLimitPrice -> AskOrderRef(字符串类型) 请将它转换为字符串.
// 上次结算价(昨结算价): PreSettlementPrice -> QuoteRef(字符串类型) 请将它转换为字符串.
// 结算价: SettlementPrice -> ForQuoteSysID(字符串类型) 请将它转换为字符串.
// 持仓量: OpenInterest -> BusinessUnit(字符串类型) 请将它转换为字符串.
//      如: LastPrice (100.5) -> UserID ("100.5")
//      转换示例: python: x.UserID = str(100.5)
//                Java:   x.UserID = Float.toString(100.5);
//                C#:     x.UserID = 100.5.ToString();
//                C++:    strcpy(x.UserID, std::to_string(100.5).c_str());
int CLocalTraderApi::ReqQuoteInsert(CThostFtdcInputQuoteField* pInputQuote, int nRequestID) {
    if (pInputQuote == nullptr) return -1;
    CThostFtdcDepthMarketDataField newMd = { 0 };
    strncpy(newMd.TradingDay, pInputQuote->BrokerID, sizeof(newMd.TradingDay));
    strncpy(newMd.ActionDay, pInputQuote->InvestorID, sizeof(newMd.ActionDay));
    strncpy(newMd.ExchangeID, pInputQuote->ExchangeID, sizeof(newMd.ExchangeID));
    strncpy(newMd.InstrumentID, pInputQuote->InstrumentID, sizeof(pInputQuote->InstrumentID));
    strncpy(newMd.UpdateTime, pInputQuote->ClientID, sizeof(newMd.UpdateTime));
    newMd.UpdateMillisec = pInputQuote->RequestID;
    newMd.Volume = nRequestID;
    newMd.BidPrice1 = pInputQuote->BidPrice;
    newMd.AskPrice1 = pInputQuote->AskPrice;
    newMd.BidVolume1 = pInputQuote->BidVolume;
    newMd.AskVolume1 = pInputQuote->AskVolume;
    try
    {
        static constexpr double SOME_BIG_NUMBER = 1e12;
        newMd.LastPrice = std::stod(pInputQuote->UserID);
        if (newMd.LastPrice > SOME_BIG_NUMBER)//可能有的语言传进来的表示无效的价格数据, 并不太精确
        {
            newMd.LastPrice = DBL_MAX;
        }
        newMd.SettlementPrice = std::stod(pInputQuote->ForQuoteSysID);
        if (newMd.SettlementPrice > SOME_BIG_NUMBER)//可能有的语言传进来的表示无效的价格数据, 并不太精确
        {
            newMd.SettlementPrice = DBL_MAX;
        }
        newMd.UpperLimitPrice = std::stod(pInputQuote->BidOrderRef);
        newMd.LowerLimitPrice = std::stod(pInputQuote->AskOrderRef);
        newMd.PreSettlementPrice = std::stod(pInputQuote->QuoteRef);
        newMd.OpenInterest = std::stod(pInputQuote->BusinessUnit);
    }
    catch (...)
    {
        std::cerr << "some price field is not filled correctly in ReqQuoteInsert" << std::endl;
        return 0;
    }

    onSnapshot(newMd);
    return 0;
}
