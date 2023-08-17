#include "stdafx.h"
#include "LocalTraderApi.h"
#include <iostream>

#define CHECK_LOGIN(p, memberName) \
    if(p==nullptr) return -1; \
    if(!m_logined) return -1; \
    if(m_userID != p->memberName || m_brokerID != p->BrokerID) return -1;

#define CHECK_LOGIN_USER(p) CHECK_LOGIN(p, UserID)

#define CHECK_LOGIN_INVESTOR(p) CHECK_LOGIN(p, InvestorID)

#define CHECK_LOGIN_ACCOUNT(p) CHECK_LOGIN(p, AccountID)

#define COMPARE_MEMBER_MATCH(a, b, memberName) \
    (strlen(a->memberName) == 0 || strcmp((a->memberName),(b.memberName)) == 0)

std::set<CLocalTraderApi::SP_TRADE_API> CLocalTraderApi::trade_api_set;
std::atomic<int> CLocalTraderApi::maxSessionID( 0 );

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

CLocalTraderApi::CLocalTraderApi(const char *pszFlowPath/* = ""*/)
	: m_bRunning(true), m_authenticated(false), m_logined(false)
    , m_orderSysID(0), m_tradeID(0), m_sessionID(0)
    , m_tradingAccount{ 0 }, m_pSpi(nullptr)
    , m_successRspInfo{ 0, "success" }, m_errorRspInfo{ -1, "error" }
{
    m_tradingAccount.PreBalance = 2e7;
    m_tradingAccount.Balance = 2e7;
#ifdef _DEBUG
    std::cout << "Welcome to LocalCTP!" << std::endl;
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
// 多头持仓的持仓盈亏 = （最新价计算得到的成本 - 持仓成本） * 合约数量乘数 * 持仓数量
// 空头持仓的持仓盈亏 = （持仓成本 - 最新价计算得到的成本） * 合约数量乘数 * 持仓数量
void CLocalTraderApi::onSnapshot(const CThostFtdcDepthMarketDataField& mdData)
{
    auto it = m_instrData.find(mdData.InstrumentID);
    if (it == m_instrData.end())
    {
        return;
    }
    // 不根据组合合约的行情快照来更新PNL
    if (it->second.ProductClass == THOST_FTDC_PC_Combination)
    {
        return;
    }
    double diffPositionProfit(0.0);
    for (auto dir : { THOST_FTDC_D_Buy, THOST_FTDC_D_Sell })
    {
        for (auto dateType : { THOST_FTDC_PSD_Today, THOST_FTDC_PSD_History })
        {
            auto posKey = CLocalTraderApi::generatePositionKey(mdData.InstrumentID,
                dir, dateType);
            // 并不更新持仓明细中的PNL
            auto itPos = m_positionData.find(posKey);
            if (itPos != m_positionData.end())
            {
                auto& PositionProfit = itPos->second.pos.PositionProfit;
                double oldPositionProfit = PositionProfit;
                PositionProfit = (dir == THOST_FTDC_D_Buy ? 1 : -1) *
                    (mdData.LastPrice * it->second.VolumeMultiple * itPos->second.pos.Position
                        - itPos->second.pos.PositionCost);
                diffPositionProfit += PositionProfit - oldPositionProfit;
            }
        }
    }
    m_tradingAccount.PositionProfit += diffPositionProfit;
    updatePNL();
}


// 计算PNL(profit and loss, 盈亏)
void CLocalTraderApi::updatePNL(bool needTotalCalc /*= false*/)
{
    if (needTotalCalc)
    {
        m_tradingAccount.PositionProfit = 0;
        m_tradingAccount.CloseProfit = 0;
        m_tradingAccount.Commission = 0;
        m_tradingAccount.CurrMargin = 0;
        m_tradingAccount.FrozenMargin = 0;
        for (const auto& P : m_positionData)
        {
            m_tradingAccount.PositionProfit += P.second.pos.PositionProfit;
            m_tradingAccount.CloseProfit += P.second.pos.CloseProfit;
            m_tradingAccount.Commission += P.second.pos.Commission;
            m_tradingAccount.CurrMargin += P.second.pos.UseMargin;
            m_tradingAccount.FrozenMargin += P.second.pos.FrozenMargin;
        }
    }
    m_tradingAccount.Balance = m_tradingAccount.PreBalance + m_tradingAccount.Deposit - m_tradingAccount.Withdraw
        + m_tradingAccount.PositionProfit + m_tradingAccount.CloseProfit - m_tradingAccount.Commission;
    m_tradingAccount.Available = m_tradingAccount.Balance - m_tradingAccount.CurrMargin - m_tradingAccount.FrozenMargin;
}

void CLocalTraderApi::updateByCancel(const CThostFtdcOrderField& o)
{
    if (THOST_FTDC_OST_Canceled != o.OrderStatus) return;

    auto it = m_instrData.find(o.InstrumentID);
    if (it == m_instrData.end())
    {
        return;
    }
    // 开仓报单已撤单,则减少冻结保证金(对应数量是未成交数量)
    if (isOpen(o.CombOffsetFlag[0]))
    {
        std::string instr = o.InstrumentID;
        auto directionT = o.Direction;

        auto handleOpen = [&]()
        {
            auto posKey = generatePositionKey(instr,
                directionT,
                THOST_FTDC_PSD_Today);
            auto itMarginRate = m_instrumentMarginRateData.find(instr);
            if (itMarginRate == m_instrumentMarginRateData.end())
            {
                return;
            }
            auto itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return;
            }
            double frozenMargin = itDepthMarketData->second.PreSettlementPrice *
                it->second.VolumeMultiple * o.VolumeTotal *
                (directionT == THOST_FTDC_D_Buy ?
                    itMarginRate->second.LongMarginRatioByMoney :
                    itMarginRate->second.ShortMarginRatioByMoney)
                + o.VolumeTotal *
                (directionT == THOST_FTDC_D_Buy ?
                    itMarginRate->second.LongMarginRatioByVolume :
                    itMarginRate->second.ShortMarginRatioByVolume);
            {
                std::lock_guard<std::mutex> posGuard(m_positionMtx);
                auto itPos = m_positionData.find(posKey);
                if (itPos == m_positionData.end())
                {
                    return;
                }
                else
                {
                    itPos->second.pos.FrozenMargin = (std::max)(
                        itPos->second.pos.FrozenMargin - frozenMargin, 0.0);

                }
            }
            m_tradingAccount.FrozenMargin = (std::max)(
                m_tradingAccount.FrozenMargin - frozenMargin, 0.0);
            updatePNL();
        };

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

                handleOpen();
            }
        }
        else
        {
            handleOpen();
        }
    }
    else// 平仓报单已撤单,则减少冻结持仓(对应数量是未成交数量)
    {
        std::string instr = o.InstrumentID;
        auto directionT = getOppositeDirection(o.Direction);

        auto handleClose = [&]()
        {
            auto posKey = generatePositionKey(instr,
                directionT,
                getDateTypeFromOffset(
                    it->second.ExchangeID, o.CombOffsetFlag[0]));
            auto itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                // 如果没找到持仓,则返回,并不插入持仓(因为持仓在开仓报单时已插入).此流程可改进
                return;
            }
            else
            {
                int& frozenPositionNum = (itPos->second.pos.PosiDirection == THOST_FTDC_PD_Long ?
                    itPos->second.pos.ShortFrozen : itPos->second.pos.LongFrozen);
                frozenPositionNum = (std::max)(
                    frozenPositionNum - o.VolumeTotal, 0);
            }
        };

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

                handleClose();
            }
        }
        else
        {
            handleClose();
        }
    }

}

void CLocalTraderApi::updateByTrade(const CThostFtdcTradeField& t)
{
    const auto* pTrade = &t;
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
    auto itMarginRate = m_instrumentMarginRateData.find(pTrade->InstrumentID);
    if (itMarginRate == m_instrumentMarginRateData.end())
    {
        return;
    }

    std::map<std::string, CThostFtdcDepthMarketDataField>::iterator itDepthMarketData;
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        itDepthMarketData = m_mdData.find(pTrade->InstrumentID);
        if (itDepthMarketData == m_mdData.end())
        {
            return;
        }
    }
    // 开仓成交时,增加一笔新的持仓明细,减少冻结保证金,更新持仓和资金
    if (isOpen(pTrade->OffsetFlag))
    {
        double frozenMargin = itDepthMarketData->second.PreSettlementPrice *
            it->second.VolumeMultiple * pTrade->Volume *
            (pTrade->Direction == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByMoney :
                itMarginRate->second.ShortMarginRatioByMoney)
            + pTrade->Volume * (pTrade->Direction == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByVolume :
                itMarginRate->second.ShortMarginRatioByVolume);
        double marginOfTrade = pTrade->Price * it->second.VolumeMultiple *
            pTrade->Volume *
            (pTrade->Direction == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByMoney :
                itMarginRate->second.ShortMarginRatioByMoney)
            + pTrade->Volume * (pTrade->Direction == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByVolume :
                itMarginRate->second.ShortMarginRatioByVolume);
        double feeOfTrade = pTrade->Price * it->second.VolumeMultiple *
            pTrade->Volume * itCommissionRate->second.OpenRatioByMoney +
            pTrade->Volume * itCommissionRate->second.OpenRatioByVolume;
        auto posKey = generatePositionKey(pTrade->InstrumentID,
            pTrade->Direction,
            THOST_FTDC_PSD_Today);
        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            auto itPos = m_positionData.find(posKey);
            if (itPos == m_positionData.end())
            {
                // 如果没找到持仓,则返回,并不插入持仓(因为持仓在开仓报单时已插入).
                return;
            }
            else
            {
                itPos->second.addPositionDetail(
                    PositionData::getPositionDetailFromOpenTrade(*pTrade));// 添加持仓明细到持仓中

                auto& pos = itPos->second.pos;
                pos.Position += pTrade->Volume;
                pos.Commission += feeOfTrade;
                pos.UseMargin += marginOfTrade;
                pos.FrozenMargin = (std::max)(
                    pos.FrozenMargin - frozenMargin, 0.0);
                pos.PositionCost +=
                    pTrade->Price * pTrade->Volume * itPos->second.volumeMultiple;//更新持仓的持仓成本
                pos.OpenVolume += pTrade->Volume;
                pos.OpenAmount +=
                    pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;
                pos.OpenCost +=
                    pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;
            }
        }
        m_tradingAccount.Commission += feeOfTrade;
        m_tradingAccount.CurrMargin += marginOfTrade;
        m_tradingAccount.FrozenMargin = (std::max)(m_tradingAccount.FrozenMargin - frozenMargin, 0.0);
        updatePNL();
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
                // 持仓明细的平仓盈亏汇总计算中,昨仓用昨结算价,今仓用开仓价
                if (strcmp(p.OpenDate, GetTradingDay()) != 0)//昨仓
                {
                    closeYesterdayVolume += tradeVolumeInThisPosDetail;
                    closeProfitOfTrade +=
                        (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1) *
                        (pTrade->Price - pos.PreSettlementPrice) *
                        tradeVolumeInThisPosDetail * itPos->second.volumeMultiple;
                }
                else//今仓
                {
                    closeTodayVolume += tradeVolumeInThisPosDetail;
                    closeProfitOfTrade +=
                        (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1) *
                        (pTrade->Price - p.OpenPrice) *
                        tradeVolumeInThisPosDetail * itPos->second.volumeMultiple;
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

            //重新统计持仓中的一些数据
            pos.Position = 0;// 持仓数量
            pos.PositionCost = 0;// 持仓成本
            pos.OpenCost = 0;// 开仓成本
            for (auto& p : itPos->second.posDetailData)
            {
                pos.Position += p.Volume;
                pos.PositionCost += p.Volume *
                    (strcmp(p.OpenDate, GetTradingDay())==0 ? p.OpenPrice : pos.PreSettlementPrice)
                    * itPos->second.volumeMultiple;// 持仓明细的持仓成本汇总计算中,昨仓用昨结算价,今仓用开仓价
                pos.OpenCost += p.Volume * p.OpenPrice * itPos->second.volumeMultiple;
            }
            pos.PositionProfit =
                (pos.PosiDirection == THOST_FTDC_PD_Long ? 1 : -1)
                * (pTrade->Price * itPos->second.volumeMultiple * pos.Position
                    - pos.PositionCost);// 持仓盈亏
            pos.CloseProfit += closeProfitOfTrade;// 平仓盈亏
            pos.Commission += feeOfTrade;// 更新持仓的手续费
            pos.UseMargin =
                pos.PositionCost *
                (pos.PosiDirection == THOST_FTDC_PD_Long ?
                    itMarginRate->second.LongMarginRatioByMoney :
                    itMarginRate->second.ShortMarginRatioByMoney)
                + pos.Position *
                (pos.PosiDirection == THOST_FTDC_PD_Long ?
                    itMarginRate->second.LongMarginRatioByVolume :
                    itMarginRate->second.ShortMarginRatioByVolume);// 更新持仓的保证金
            int& frozenPositionNum = (pos.PosiDirection == THOST_FTDC_PD_Long ?
                pos.ShortFrozen : pos.LongFrozen);
            frozenPositionNum = (std::max)(
                frozenPositionNum - pTrade->Volume, 0);//更新持仓的冻结持仓
            pos.CloseVolume += pTrade->Volume;//更新持仓的平仓量
            pos.CloseAmount +=
                pTrade->Volume * pTrade->Price * itPos->second.volumeMultiple;//更新持仓的平仓金额
            // 汇总计算资金
            updatePNL(true);
        }
    }
}

///创建TraderApi
///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
///@return 创建出的UserApi
CThostFtdcTraderApi* CThostFtdcTraderApi::CreateFtdcTraderApi(const char *pszFlowPath/* = ""*/) {
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

///初始化
///@remark 初始化运行环境,只有调用后,接口才开始工作
void CLocalTraderApi::Init() {
	// 从当前目录(或环境变量中的目录)的 instrument.csv 中读取合约信息
    std::ifstream ifs("instrument.csv");
    if (!ifs.is_open())
    {
        return;
    }
    std::string singleLine;
    if (!std::getline(ifs, singleLine))//第一行是表头
    {
        return;
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
    std::cout << "Total instrument count: " << m_instrData.size() << std::endl;
#endif

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

    //临时措施:将所有合约的保证金率和手续费率初始化(保证金率全部为10%,手续费全部为1元每手)
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

    if (m_pSpi == nullptr) return;

    m_pSpi->OnFrontConnected();
    return;
}

///等待接口线程结束运行
///@return 线程退出代码
int CLocalTraderApi::Join() {
    // Do nothing
	return 0;
}

///获取当前交易日
///@retrun 获取到的交易日
///@remark 只有登录成功后,才能得到正确的交易日
const char* CLocalTraderApi::GetTradingDay() {
    // use ( now time + 4 hours) as trading date,
    // it will consider the weekend, but not the holiday.
    // for example:
    // 1. 2023-08-07 10:00 -> 2023-08-07 14:00 -> return "20230807"
    // 2. 2023-08-07 20:00 -> 2023-08-08 02:00 -> return "20230808"
    // 3. 2023-08-04 20:00(Fri) -> 2023-08-05 02:00(Sat) -> 2023-08-07 02:00(Mon) -> return "20230807"
    static std::string tradingDay;
    auto checkTime = CLeeDateTime::now() + CLeeDateTimeSpan(0, 4, 0, 0);
    if (checkTime.GetDayOfWeek() == 6)
    {
        checkTime += CLeeDateTimeSpan(2, 0, 0, 0);
    }
    tradingDay = checkTime.Format("%Y%m%d");
    return tradingDay.c_str();
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
    std::string instrumentID;
    if (strlen(md->InstrumentID) == 0)
    {
        instrumentID = md->reserve1; // the old InstrumentID in struct before 6.5.1
    }
    else
    {
        instrumentID = md->InstrumentID; // the new InstrumentID in struct since 6.5.1
    }
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        m_mdData[instrumentID] = *md;
    }
    {
        for (auto& contionalOrderPair : m_contionalOrders)
        {
            auto& contionalOrder = contionalOrderPair.second;
            if (strcmp(md->InstrumentID, contionalOrder.rtnOrder.InstrumentID) != 0
                || contionalOrder.rtnOrder.OrderStatus == THOST_FTDC_OST_Touched)
            {
                continue;
            }
            auto isPriceValid = [](const bool price) {return LT(price, DBL_MAX); };
            auto matchContion = [&]() -> bool {
                switch (contionalOrder.rtnOrder.ContingentCondition)
                {
                case THOST_FTDC_CC_LastPriceGreaterThanStopPrice:
                    return isPriceValid(md->LastPrice) && GT(md->LastPrice, contionalOrder.rtnOrder.StopPrice);
                case THOST_FTDC_CC_LastPriceGreaterEqualStopPrice:
                    return isPriceValid(md->LastPrice) && GE(md->LastPrice, contionalOrder.rtnOrder.StopPrice);
                case THOST_FTDC_CC_LastPriceLesserThanStopPrice:
                    return isPriceValid(md->LastPrice) && LT(md->LastPrice, contionalOrder.rtnOrder.StopPrice);
                case THOST_FTDC_CC_LastPriceLesserEqualStopPrice:
                    return isPriceValid(md->LastPrice) && LE(md->LastPrice, contionalOrder.rtnOrder.StopPrice);
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
            ReqOrderInsertImpl(&contionalOrder.inputOrder, 0, contionalOrder.rtnOrder.OrderSysID);
        }

        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        // 此时订单数据中已包含刚才条件单触发后产生的新报单O(∩_∩)O
        for (auto& o : m_orderData)
        {
            auto& order = o.second;
            if (instrumentID != order.rtnOrder.InstrumentID || order.isDone())
            {
                continue;
            }
            if (isMatchTrade(order.rtnOrder.Direction, order.rtnOrder.LimitPrice, *md))
            {
                order.handleTrade(order.rtnOrder.LimitPrice, order.rtnOrder.VolumeTotal);
            }
        }
    }

    onSnapshot(*md);
}

///注册回调接口
///@param pSpi 派生自回调接口类的实例
void CLocalTraderApi::RegisterSpi(CThostFtdcTraderSpi *pSpi) {
    m_pSpi = pSpi;
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
    if (pReqAuthenticateField == nullptr) return -1;
    CThostFtdcRspAuthenticateField RspAuthenticateField = { 0 };
    memcpy(&RspAuthenticateField, pReqAuthenticateField, sizeof(CThostFtdcRspAuthenticateField));
    if (strlen(pReqAuthenticateField->UserID) == 0 ||
        strlen(pReqAuthenticateField->BrokerID) == 0)
    {
        if (m_pSpi == nullptr) return 0;
        m_pSpi->OnRspAuthenticate(&RspAuthenticateField, setErrorMsgAndGetRspInfo(ErrMsgUserInfoIsEmpty), nRequestID, true);
        return 0;
    }
    if ((!m_userID.empty() && m_userID != pReqAuthenticateField->UserID) ||
        (!m_brokerID.empty() && m_brokerID != pReqAuthenticateField->BrokerID))
    {
        if (m_pSpi == nullptr) return 0;
        m_pSpi->OnRspAuthenticate(&RspAuthenticateField, setErrorMsgAndGetRspInfo(ErrMsgUserInfoNotSameAsLastTime), nRequestID, true);
        return 0;
    }

    m_authenticated = true;
    m_userID = pReqAuthenticateField->UserID;
    m_brokerID = pReqAuthenticateField->BrokerID;
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspAuthenticate(&RspAuthenticateField, &m_successRspInfo, nRequestID, true);
    return 0;
}

///注册用户终端信息，用于中继服务器多连接模式
///需要在终端认证成功后，用户登录前调用该接口
int CLocalTraderApi::RegisterUserSystemInfo(CThostFtdcUserSystemInfoField *pUserSystemInfo) { return -1; }

///上报用户终端信息，用于中继服务器操作员登录模式
///操作员登录后，可以多次调用该接口上报客户信息
int CLocalTraderApi::SubmitUserSystemInfo(CThostFtdcUserSystemInfoField *pUserSystemInfo) { return -1; }

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
        if (m_pSpi == nullptr) return 0;
        m_pSpi->OnRspUserLogin(&RspUserLogin, setErrorMsgAndGetRspInfo(ErrMsgNotAuth), nRequestID, true);
        return 0;
    }
    if (m_userID != pReqUserLoginField->UserID || m_brokerID != pReqUserLoginField->BrokerID)
    {
        if (m_pSpi == nullptr) return 0;
        m_pSpi->OnRspUserLogin(&RspUserLogin, setErrorMsgAndGetRspInfo(ErrMsgUserInfoNotSameAsAuth), nRequestID, true);
        return 0;
    }
    m_logined = true;
    if (m_pSpi == nullptr) return 0;
    strncpy(RspUserLogin.TradingDay, GetTradingDay(), sizeof(RspUserLogin.TradingDay));
    strncpy(RspUserLogin.LoginTime, CLeeDateTime::GetCurrentTime().Format("%H:%M:%S").c_str(),
        sizeof(RspUserLogin.LoginTime));
    strncpy(RspUserLogin.SHFETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.SHFETime));
    strncpy(RspUserLogin.DCETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.DCETime));
    strncpy(RspUserLogin.CZCETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.CZCETime));
    strncpy(RspUserLogin.FFEXTime, RspUserLogin.LoginTime, sizeof(RspUserLogin.FFEXTime));
    strncpy(RspUserLogin.INETime, RspUserLogin.LoginTime, sizeof(RspUserLogin.INETime));
    strncpy(RspUserLogin.SystemName, "LocalCTP", sizeof(RspUserLogin.SystemName));
    strncpy(RspUserLogin.MaxOrderRef, "1", sizeof(RspUserLogin.MaxOrderRef));
    m_sessionID = maxSessionID++;
    RspUserLogin.SessionID = m_sessionID;
    m_pSpi->OnRspUserLogin(&RspUserLogin, &m_successRspInfo, nRequestID, true);
    return 0;
}

///登出请求
//登出后账户数据(含订单,持仓,资金等)仍然保留,仍然可以传入行情数据并更新资金等数据,因此实际不需要登出
int CLocalTraderApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID) {
    CHECK_LOGIN_USER(pUserLogout);

    m_authenticated = false;
    m_logined = false;
    if (m_pSpi == nullptr) return 0;
    CThostFtdcUserLogoutField RspUserLogout = { 0 };
    strncpy(RspUserLogout.UserID, pUserLogout->UserID, sizeof(RspUserLogout.BrokerID));
    strncpy(RspUserLogout.BrokerID, pUserLogout->BrokerID, sizeof(RspUserLogout.BrokerID));
    m_pSpi->OnRspUserLogout(&RspUserLogout, &m_successRspInfo, nRequestID, true);
#if 0
    //另一种方案:不允许登出. "上了车还想跑? 车门已焊死!"
    m_pSpi->OnRspUserLogout(&RspUserLogout, &setErrorMsgAndGetRspInfo("Logout is not supported in this system."),
        nRequestID, true);
#endif
    return 0;
}

///用户口令更新请求
int CLocalTraderApi::ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) {
    CHECK_LOGIN_USER(pUserPasswordUpdate);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspUserPasswordUpdate(nullptr,
        setErrorMsgAndGetRspInfo("Update password is not supported in this system."),
        nRequestID, true);
    return 0;
}

///资金账户口令更新请求
int CLocalTraderApi::ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) {
    CHECK_LOGIN_ACCOUNT(pTradingAccountPasswordUpdate);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspTradingAccountPasswordUpdate(nullptr,
        setErrorMsgAndGetRspInfo("Update password is not supported in this system."),
        nRequestID, true);
    return 0;
}

///查询用户当前支持的认证模式
int CLocalTraderApi::ReqUserAuthMethod(CThostFtdcReqUserAuthMethodField *pReqUserAuthMethod, int nRequestID) { return -1; }

///用户发出获取图形验证码请求
int CLocalTraderApi::ReqGenUserCaptcha(CThostFtdcReqGenUserCaptchaField *pReqGenUserCaptcha, int nRequestID) { return -1; }

///用户发出获取短信验证码请求
int CLocalTraderApi::ReqGenUserText(CThostFtdcReqGenUserTextField *pReqGenUserText, int nRequestID) { return -1; }

///用户发出带有图片验证码的登陆请求
int CLocalTraderApi::ReqUserLoginWithCaptcha(CThostFtdcReqUserLoginWithCaptchaField *pReqUserLoginWithCaptcha, int nRequestID) { return -1; }

///用户发出带有短信验证码的登陆请求
int CLocalTraderApi::ReqUserLoginWithText(CThostFtdcReqUserLoginWithTextField *pReqUserLoginWithText, int nRequestID) { return -1; }

///用户发出带有动态口令的登陆请求
int CLocalTraderApi::ReqUserLoginWithOTP(CThostFtdcReqUserLoginWithOTPField *pReqUserLoginWithOTP, int nRequestID) { return -1; }

///报单录入请求
int CLocalTraderApi::ReqOrderInsert(CThostFtdcInputOrderField* pInputOrder, int nRequestID) {
    return ReqOrderInsertImpl(pInputOrder, nRequestID);
}
int CLocalTraderApi::ReqOrderInsertImpl(CThostFtdcInputOrderField * pInputOrder, int nRequestID,
    std::string relativeOrderSysID /*= std::string()*/) {
    CHECK_LOGIN_USER(pInputOrder);

    const auto sendRejectOrder = [&](const char* errMsg) {
        m_pSpi->OnRspOrderInsert(pInputOrder, setErrorMsgAndGetRspInfo(errMsg), nRequestID, true);
        m_pSpi->OnErrRtnOrderInsert(pInputOrder, setErrorMsgAndGetRspInfo(errMsg));
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

    std::map<std::string, CThostFtdcDepthMarketDataField>::iterator itMd;
    {
        std::lock_guard<std::mutex> mdGuard(m_mdMtx);
        itMd = m_mdData.find(pInputOrder->InstrumentID);
        if (itMd == m_mdData.end())
        {
            sendRejectOrder((std::string(ErrMsg_NoMarketData) + pInputOrder->InstrumentID).c_str());
            return 0;
        }
    }

    if (isConditionalType(pInputOrder->ContingentCondition))
    {
        OrderData x(this, *pInputOrder, true);
        m_contionalOrders.emplace(x.rtnOrder.OrderSysID, x);
        return 0;
    }

    std::string instr = pInputOrder->InstrumentID;
    auto directionT = pInputOrder->Direction;
    const int orderNum = pInputOrder->VolumeTotalOriginal;
    double totalFrozenMargin = 0;

    auto handleOpen = [&](bool preCheck) -> std::pair<bool, std::string>
    {
        // 如果是开仓报单,增加冻结保证金
        auto posKey = generatePositionKey(instr,
            directionT,
            THOST_FTDC_PSD_Today);
        auto itMarginRate = m_instrumentMarginRateData.find(instr);
        if (itMarginRate == m_instrumentMarginRateData.end())
        {
            return std::make_pair(false, ErrMsg_INSTRUMENT_MARGINRATE_NOT_FOUND + instr);
        }
        std::map<std::string, CThostFtdcDepthMarketDataField>::iterator itDepthMarketData;
        if (itInstr->second.ProductClass == THOST_FTDC_PC_Combination)
        {
            std::lock_guard<std::mutex> mdGuard(m_mdMtx);
            itDepthMarketData = m_mdData.find(instr);
            if (itDepthMarketData == m_mdData.end())
            {
                return std::make_pair(false, ErrMsg_NoMarketData + instr);
            }
        }
        else //如果不是组合合约,则无需再次查找行情了
        {
            itDepthMarketData = itMd;
        }
        // 冻结保证金和保证金计算使用的价格,不同期货公司不一样.
        // 可以参见TThostFtdcMarginPriceTypeType类型取值说明.
        // 可通过查询经纪公司交易参数获得
        // (请求查询函数ReqQryBrokerTradingParams, 响应函数OnRspQryBrokerTradingParams)
        // 本DEMO以昨结算价作为计算冻结保证金的基准价格,
        // 而以昨结算价(对昨仓)和开仓成交价格(对今仓)作为计算持仓保证金时的基准价格
        double frozenMargin = itDepthMarketData->second.PreSettlementPrice *
            itInstr->second.VolumeMultiple * orderNum *
            (directionT == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByMoney
                : itMarginRate->second.ShortMarginRatioByMoney)
            +
            orderNum * (directionT == THOST_FTDC_D_Buy ?
                itMarginRate->second.LongMarginRatioByVolume
                : itMarginRate->second.ShortMarginRatioByVolume);
        totalFrozenMargin += frozenMargin;

        {
            std::lock_guard<std::mutex> posGuard(m_positionMtx);
            auto itPos = m_positionData.find(posKey);
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
                tempPos.pos.SettlementPrice = itDepthMarketData->second.LastPrice;
                strncpy(tempPos.pos.TradingDay, GetTradingDay(), sizeof(tempPos.pos.TradingDay));
                if (!preCheck)
                {
                    tempPos.pos.FrozenMargin = frozenMargin;// (因为开仓未成交而)冻结的保证金
                }
                tempPos.pos.PosiDirection = (directionT == THOST_FTDC_D_Buy ? THOST_FTDC_PD_Long : THOST_FTDC_PD_Short);// 持仓方向
                tempPos.pos.PositionDate = THOST_FTDC_PSD_Today;// 持仓日期类型(今仓)
                m_positionData.emplace(posKey, tempPos);
            }
            else
            {
                if (!preCheck)
                {
                    itPos->second.pos.FrozenMargin += frozenMargin;
                }
            }
        }
        if (preCheck)
        {
            double newAvailable = m_tradingAccount.Balance - m_tradingAccount.CurrMargin - m_tradingAccount.FrozenMargin
                - totalFrozenMargin;
            if (LTZ(newAvailable))
            {
                return std::make_pair(false, ERRMSG_AVAILABLE_NOT_ENOUGH);;
            }
        }
        else
        {
            m_tradingAccount.FrozenMargin += frozenMargin;
            updatePNL();
        }
        return std::make_pair(true, std::string());
    };
    auto handleClose = [&](bool preCheck) -> std::pair<bool, std::string> {
        // 如果是平仓报单,则校验可平仓数量和增加冻结持仓数量
        auto posKey = generatePositionKey(instr,
            directionT,
            getDateTypeFromOffset(itInstr->second.ExchangeID, pInputOrder->CombOffsetFlag[0]));

        std::lock_guard<std::mutex> posGuard(m_positionMtx);
        auto itPos = m_positionData.find(posKey);
        if (itPos == m_positionData.end())
        {
            return std::make_pair(false, ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH + std::string("0")
                + " on " + instr);
        }
        else
        {
            int& frozenPositionNum = (itPos->second.pos.PosiDirection == THOST_FTDC_PD_Long ?
                itPos->second.pos.ShortFrozen : itPos->second.pos.LongFrozen);
            const auto closable = itPos->second.pos.Position - frozenPositionNum;
            if (closable < orderNum)
            {
                return std::make_pair(false, 
                    (isSpecialExchange(itInstr->second.ExchangeID) && pInputOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday) ?
                        ERRMSG_AVAILABLE_POSITION_NOT_ENOUGH : ERRMSG_AVAILABLE_TODAY_POSITION_NOT_ENOUGH
                        + std::to_string(closable) + " on " + instr);
            }
            if (!preCheck)
            {
                frozenPositionNum += orderNum;
            }
        }
        return std::make_pair(true, std::string());
    };

    auto doRiskCheck = [&](bool preCheck) -> std::pair<bool, std::string>
    {
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

                    auto handleOpenRet = handleOpen(preCheck);
                    if (!handleOpenRet.first)
                    {
                        return handleOpenRet;;
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

                    auto handleCloseRet = handleClose(preCheck);
                    if (!handleCloseRet.first)
                    {
                        return handleCloseRet;
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

    std::map<int, OrderData>::iterator itOrder;
    {
        int OrderRef = atoi(pInputOrder->OrderRef);
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        if (strlen(pInputOrder->OrderRef) != 0)// check the OrderRef if OrderRef in order is not empty
        {
            if (!m_orderData.empty() && OrderRef < (--m_orderData.end())->first)
            {
                sendRejectOrder(ErrMsgDuplicateOrder);
                return 0;
            }
        }
        OrderData x(this, *pInputOrder, false, relativeOrderSysID);
        bool emplaceSuccess(false);
        std::tie(itOrder, emplaceSuccess) = m_orderData.emplace(OrderRef, x);
        if (!emplaceSuccess)
        {
            sendRejectOrder(ErrMsgDuplicateOrder);
            return 0;
        }
    }

    checkRet = doRiskCheck(false);//更新风控值
    /*if (!checkRet.first) //此前已经校验过,不会再为false
    {
        sendRejectOrder(checkRet.second.c_str());
        return 0;
    }*/
    {
        if(isMatchTrade(pInputOrder->Direction, pInputOrder->LimitPrice, itMd->second))
        {
            itOrder->second.handleTrade(pInputOrder->LimitPrice, orderNum);
            return 0;
        }
        else
        {
            // 判断是否是IOC订单
            // 可能有的交易所不是这样判断,但本系统以这种方式来统一判断处理
            auto isIOCOrder = [&]() {
                return pInputOrder->TimeCondition == THOST_FTDC_TC_IOC && pInputOrder->VolumeCondition == THOST_FTDC_VC_CV;
            };
            if (isIOCOrder())
            {
                itOrder->second.handleCancel(false);
                return 0;
            }
        }
    }

    
    return 0;
}

///预埋单录入请求
int CLocalTraderApi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID) {
    CHECK_LOGIN_USER(pParkedOrder);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspParkedOrderInsert(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true);
    return 0;
}

///预埋撤单录入请求
int CLocalTraderApi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID) {
    CHECK_LOGIN_USER(pParkedOrderAction);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspParkedOrderAction(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true);
    return 0;
}

///报单操作请求
int CLocalTraderApi::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID) {
    CHECK_LOGIN_USER(pInputOrderAction);

    if (pInputOrderAction->ActionFlag != THOST_FTDC_AF_Delete)
    {
        if (m_pSpi == nullptr) return 0;
        m_pSpi->OnRspOrderAction(pInputOrderAction, setErrorMsgAndGetRspInfo(ErrMsg_NotSupportModifyOrder), nRequestID, true);
        return 0;
    }
    int OrderRef = atoi(pInputOrderAction->OrderRef);
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        auto itOrder = m_orderData.find(OrderRef);
        // first, we find order by "OrderRef + FrontID + SessionID" (and, the InstrumentID must be valid)
        if (itOrder != m_orderData.end())
        {
            auto& order = itOrder->second;
            if (order.isDone() ||
                order.rtnOrder.FrontID != pInputOrderAction->FrontID ||
                order.rtnOrder.SessionID != pInputOrderAction->SessionID ||
                strcmp( order.rtnOrder.InstrumentID, pInputOrderAction->InstrumentID) != 0)
            {
                if (m_pSpi == nullptr) return 0;
                m_pSpi->OnRspOrderAction(pInputOrderAction, setErrorMsgAndGetRspInfo(
                    order.isDone() ? ErrMsg_AlreadyDoneOrder : ErrMsg_NotExistOrder),
                    nRequestID, true);
                return 0;
            }
            else
            {
                order.handleCancel();
                return 0;
            }
        }
        else // second, we find order by "OrderSysID + ExchangeID"
        {
            for (auto& orderPair : m_orderData)
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
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspOrderAction(pInputOrderAction, setErrorMsgAndGetRspInfo(ErrMsg_NotExistOrder), nRequestID, true);
    return 0;
}

///查询最大报单数量请求
int CLocalTraderApi::ReqQryMaxOrderVolume(CThostFtdcQryMaxOrderVolumeField *pQryMaxOrderVolume, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryMaxOrderVolume);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspQryMaxOrderVolume(nullptr,
        setErrorMsgAndGetRspInfo("Query MaxOrderVolume is not supported in this system."),
        nRequestID, true);
    return 0;
}

///投资者结算结果确认
int CLocalTraderApi::ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pSettlementInfoConfirm);
    if (m_pSpi == nullptr) return 0;
    CThostFtdcSettlementInfoConfirmField SettlementInfoConfirm = *pSettlementInfoConfirm;
    m_pSpi->OnRspSettlementInfoConfirm(&SettlementInfoConfirm, &m_successRspInfo, nRequestID, true);
    return 0;
}

///请求删除预埋单
int CLocalTraderApi::ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pRemoveParkedOrder);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspRemoveParkedOrder(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true);
    return 0;
}

///请求删除预埋撤单
int CLocalTraderApi::ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pRemoveParkedOrderAction);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspRemoveParkedOrderAction(nullptr,
        setErrorMsgAndGetRspInfo("Parked order is not supported in this system."),
        nRequestID, true);
    return 0;
}

///执行宣告录入请求
int CLocalTraderApi::ReqExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, int nRequestID) { return -1; }

///执行宣告操作请求
int CLocalTraderApi::ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, int nRequestID) { return -1; }

///询价录入请求
int CLocalTraderApi::ReqForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, int nRequestID) { return -1; }

///报价录入请求
int CLocalTraderApi::ReqQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, int nRequestID) { return -1; }

///报价操作请求
int CLocalTraderApi::ReqQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, int nRequestID) { return -1; }

///批量报单操作请求
int CLocalTraderApi::ReqBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, int nRequestID) { return -1; }

///期权自对冲录入请求
int CLocalTraderApi::ReqOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, int nRequestID) { return -1; }

///期权自对冲操作请求
int CLocalTraderApi::ReqOptionSelfCloseAction(CThostFtdcInputOptionSelfCloseActionField *pInputOptionSelfCloseAction, int nRequestID) { return -1; }

///申请组合录入请求
int CLocalTraderApi::ReqCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, int nRequestID) { return -1; }

///请求查询报单
int CLocalTraderApi::ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryOrder);
    if (m_pSpi == nullptr) return 0;
    std::vector<CThostFtdcOrderField*> v;
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        for (auto& o : m_orderData)
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
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_pSpi->OnRspQryOrder(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryOrder(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询成交
int CLocalTraderApi::ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryTrade);
    if (m_pSpi == nullptr) return 0;
    std::vector<CThostFtdcTradeField*> v;
    {
        std::lock_guard<std::mutex> orderGuard(m_orderMtx);
        for (auto& o : m_orderData)
        {
            for (auto& t : o.second.rtnTrades)
            {
                if (COMPARE_MEMBER_MATCH(pQryTrade, t, ExchangeID) &&
                    COMPARE_MEMBER_MATCH(pQryTrade, t, TradeID) &&
                    COMPARE_MEMBER_MATCH(pQryTrade, t, InstrumentID))
                {
                    v.emplace_back(&t);
                }
            }
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_pSpi->OnRspQryTrade(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryTrade(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询投资者持仓
int CLocalTraderApi::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestorPosition);
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryInvestorPosition(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryInvestorPosition(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询资金账户
int CLocalTraderApi::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryTradingAccount);
    if (m_pSpi == nullptr) return 0;
    strncpy(m_tradingAccount.BrokerID, m_brokerID.c_str(), sizeof(m_tradingAccount.BrokerID));
    strncpy(m_tradingAccount.AccountID, m_userID.c_str(), sizeof(m_tradingAccount.AccountID));
    strncpy(m_tradingAccount.TradingDay, GetTradingDay(), sizeof(m_tradingAccount.TradingDay));
    m_pSpi->OnRspQryTradingAccount(&m_tradingAccount, &m_successRspInfo, nRequestID, true);
    return 0;
}

///请求查询投资者
int CLocalTraderApi::ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestor);
    if (m_pSpi == nullptr) return 0;
    CThostFtdcInvestorField Investor = { 0 };
    strncpy(Investor.InvestorID, pQryInvestor->InvestorID, sizeof(Investor.InvestorID));
    strncpy(Investor.BrokerID, pQryInvestor->BrokerID, sizeof(Investor.BrokerID));
    Investor.IdentifiedCardType = THOST_FTDC_ICT_OtherCard;
    strncpy(Investor.IdentifiedCardNo, "QQ1005018695", sizeof(Investor.IdentifiedCardNo));
    Investor.IsActive = 1;
    m_pSpi->OnRspQryInvestor(&Investor, &m_successRspInfo, nRequestID, true);
    return 0;
}

///请求查询交易编码
int CLocalTraderApi::ReqQryTradingCode(CThostFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) { return -1; }

///请求查询合约保证金率
int CLocalTraderApi::ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID) {
    if (pQryInstrumentMarginRate == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryInstrumentMarginRate(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryInstrumentMarginRate(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询合约手续费率
int CLocalTraderApi::ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) {
    if (pQryInstrumentCommissionRate == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryInstrumentCommissionRate(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryInstrumentCommissionRate(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询交易所
int CLocalTraderApi::ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID) {
    if (pQryExchange == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryExchange(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryInstrument(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询产品
int CLocalTraderApi::ReqQryProduct(CThostFtdcQryProductField *pQryProduct, int nRequestID) {
    if (pQryProduct == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryProduct(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryProduct(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询合约
int CLocalTraderApi::ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID) {
    if (pQryInstrument == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryInstrument(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryInstrument(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询行情
int CLocalTraderApi::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) {
    if (pQryDepthMarketData == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryDepthMarketData(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryDepthMarketData(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询投资者结算结果
int CLocalTraderApi::ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQrySettlementInfo);
    if (m_pSpi == nullptr) return 0;
    m_pSpi->OnRspQrySettlementInfo(nullptr, &m_successRspInfo, nRequestID, true);
    return 0;
}

///请求查询转帐银行
int CLocalTraderApi::ReqQryTransferBank(CThostFtdcQryTransferBankField *pQryTransferBank, int nRequestID) { return -1; }

///请求查询投资者持仓明细
int CLocalTraderApi::ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQryInvestorPositionDetail);
    if (m_pSpi == nullptr) return 0;
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
        m_pSpi->OnRspQryInvestorPositionDetail(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryTrade(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求查询客户通知
int CLocalTraderApi::ReqQryNotice(CThostFtdcQryNoticeField *pQryNotice, int nRequestID) { return -1; }

///请求查询结算信息确认
int CLocalTraderApi::ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) {
    CHECK_LOGIN_INVESTOR(pQrySettlementInfoConfirm);
    if (m_pSpi == nullptr) return 0;
    CThostFtdcSettlementInfoConfirmField SettlementInfoConfirm = { 0 };
    strncpy(SettlementInfoConfirm.BrokerID, pQrySettlementInfoConfirm->BrokerID, sizeof(SettlementInfoConfirm.BrokerID));
    strncpy(SettlementInfoConfirm.InvestorID, pQrySettlementInfoConfirm->InvestorID, sizeof(SettlementInfoConfirm.InvestorID));
    strncpy(SettlementInfoConfirm.ConfirmDate, GetTradingDay(), sizeof(SettlementInfoConfirm.ConfirmDate));
    m_pSpi->OnRspQrySettlementInfoConfirm(&SettlementInfoConfirm, &m_successRspInfo, nRequestID, true);
    return 0;
}

///请求查询投资者持仓明细
int CLocalTraderApi::ReqQryInvestorPositionCombineDetail(CThostFtdcQryInvestorPositionCombineDetailField *pQryInvestorPositionCombineDetail, int nRequestID) { return -1; }

///请求查询保证金监管系统经纪公司资金账户密钥
int CLocalTraderApi::ReqQryCFMMCTradingAccountKey(CThostFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID) { return -1; }

///请求查询仓单折抵信息
int CLocalTraderApi::ReqQryEWarrantOffset(CThostFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID) { return -1; }

///请求查询投资者品种/跨品种保证金
int CLocalTraderApi::ReqQryInvestorProductGroupMargin(CThostFtdcQryInvestorProductGroupMarginField *pQryInvestorProductGroupMargin, int nRequestID) { return -1; }

///请求查询交易所保证金率
int CLocalTraderApi::ReqQryExchangeMarginRate(CThostFtdcQryExchangeMarginRateField *pQryExchangeMarginRate, int nRequestID) { return -1; }

///请求查询交易所调整保证金率
int CLocalTraderApi::ReqQryExchangeMarginRateAdjust(CThostFtdcQryExchangeMarginRateAdjustField *pQryExchangeMarginRateAdjust, int nRequestID) { return -1; }

///请求查询汇率
int CLocalTraderApi::ReqQryExchangeRate(CThostFtdcQryExchangeRateField *pQryExchangeRate, int nRequestID) { return -1; }

///请求查询二级代理操作员银期权限
int CLocalTraderApi::ReqQrySecAgentACIDMap(CThostFtdcQrySecAgentACIDMapField *pQrySecAgentACIDMap, int nRequestID) { return -1; }

///请求查询产品报价汇率
int CLocalTraderApi::ReqQryProductExchRate(CThostFtdcQryProductExchRateField *pQryProductExchRate, int nRequestID) { return -1; }

///请求查询产品组
int CLocalTraderApi::ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID) { return -1; }

///请求查询做市商合约手续费率
int CLocalTraderApi::ReqQryMMInstrumentCommissionRate(CThostFtdcQryMMInstrumentCommissionRateField *pQryMMInstrumentCommissionRate, int nRequestID) { return -1; }

///请求查询做市商期权合约手续费
int CLocalTraderApi::ReqQryMMOptionInstrCommRate(CThostFtdcQryMMOptionInstrCommRateField *pQryMMOptionInstrCommRate, int nRequestID) { return -1; }

///请求查询报单手续费
int CLocalTraderApi::ReqQryInstrumentOrderCommRate(CThostFtdcQryInstrumentOrderCommRateField *pQryInstrumentOrderCommRate, int nRequestID) { return -1; }

///请求查询资金账户
int CLocalTraderApi::ReqQrySecAgentTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) { return -1; }

///请求查询二级代理商资金校验模式
int CLocalTraderApi::ReqQrySecAgentCheckMode(CThostFtdcQrySecAgentCheckModeField *pQrySecAgentCheckMode, int nRequestID) { return -1; }

///请求查询二级代理商信息
int CLocalTraderApi::ReqQrySecAgentTradeInfo(CThostFtdcQrySecAgentTradeInfoField *pQrySecAgentTradeInfo, int nRequestID) { return -1; }

///请求查询期权交易成本
int CLocalTraderApi::ReqQryOptionInstrTradeCost(CThostFtdcQryOptionInstrTradeCostField *pQryOptionInstrTradeCost, int nRequestID) { return -1; }

///请求查询期权合约手续费
int CLocalTraderApi::ReqQryOptionInstrCommRate(CThostFtdcQryOptionInstrCommRateField *pQryOptionInstrCommRate, int nRequestID) { return -1; }

///请求查询执行宣告
int CLocalTraderApi::ReqQryExecOrder(CThostFtdcQryExecOrderField *pQryExecOrder, int nRequestID) { return -1; }

///请求查询询价
int CLocalTraderApi::ReqQryForQuote(CThostFtdcQryForQuoteField *pQryForQuote, int nRequestID) { return -1; }

///请求查询报价
int CLocalTraderApi::ReqQryQuote(CThostFtdcQryQuoteField *pQryQuote, int nRequestID) { return -1; }

///请求查询期权自对冲
int CLocalTraderApi::ReqQryOptionSelfClose(CThostFtdcQryOptionSelfCloseField *pQryOptionSelfClose, int nRequestID) { return -1; }

///请求查询投资单元
int CLocalTraderApi::ReqQryInvestUnit(CThostFtdcQryInvestUnitField *pQryInvestUnit, int nRequestID) { return -1; }

///请求查询组合合约安全系数
int CLocalTraderApi::ReqQryCombInstrumentGuard(CThostFtdcQryCombInstrumentGuardField *pQryCombInstrumentGuard, int nRequestID) { return -1; }

///请求查询申请组合
int CLocalTraderApi::ReqQryCombAction(CThostFtdcQryCombActionField *pQryCombAction, int nRequestID) { return -1; }

///请求查询转帐流水
int CLocalTraderApi::ReqQryTransferSerial(CThostFtdcQryTransferSerialField *pQryTransferSerial, int nRequestID) { return -1; }

///请求查询银期签约关系
int CLocalTraderApi::ReqQryAccountregister(CThostFtdcQryAccountregisterField *pQryAccountregister, int nRequestID) { return -1; }

///请求查询签约银行
int CLocalTraderApi::ReqQryContractBank(CThostFtdcQryContractBankField *pQryContractBank, int nRequestID) { return -1; }

///请求查询预埋单
int CLocalTraderApi::ReqQryParkedOrder(CThostFtdcQryParkedOrderField *pQryParkedOrder, int nRequestID) { return -1; }

///请求查询预埋撤单
int CLocalTraderApi::ReqQryParkedOrderAction(CThostFtdcQryParkedOrderActionField *pQryParkedOrderAction, int nRequestID) { return -1; }

///请求查询交易通知
int CLocalTraderApi::ReqQryTradingNotice(CThostFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID) { return -1; }

///请求查询经纪公司交易参数
int CLocalTraderApi::ReqQryBrokerTradingParams(CThostFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID) { return -1; }

///请求查询经纪公司交易算法
int CLocalTraderApi::ReqQryBrokerTradingAlgos(CThostFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID) { return -1; }

///请求查询监控中心用户令牌
int CLocalTraderApi::ReqQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, int nRequestID) { return -1; }

///期货发起银行资金转期货请求
int CLocalTraderApi::ReqFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) { return -1; }

///期货发起期货资金转银行请求
int CLocalTraderApi::ReqFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) { return -1; }

///期货发起查询银行余额请求
int CLocalTraderApi::ReqQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, int nRequestID) { return -1; }

///请求查询分类合约
int CLocalTraderApi::ReqQryClassifiedInstrument(CThostFtdcQryClassifiedInstrumentField *pQryClassifiedInstrument, int nRequestID) {
    if (pQryClassifiedInstrument == nullptr || !m_logined) return -1;
    if (m_pSpi == nullptr) return 0;
    std::vector<CThostFtdcInstrumentField*> v;
    v.reserve(1000); // maybe less than this count ?
    for (auto& instrPair : m_instrData)
    {
        CThostFtdcInstrumentField& instr = instrPair.second;
        auto matchClassType = [&]() -> bool {
            switch (pQryClassifiedInstrument->ClassType)
            {
            case THOST_FTDC_INS_ALL:
                return true;
            case THOST_FTDC_INS_FUTURE:
                return instr.ProductClass == THOST_FTDC_PC_Futures || instr.ProductClass == THOST_FTDC_PC_Spot ||
                    instr.ProductClass == THOST_FTDC_PC_EFP || instr.ProductClass == THOST_FTDC_PC_TAS ||
                    instr.ProductClass == THOST_FTDC_PC_MI;
            case THOST_FTDC_INS_OPTION:
                return instr.ProductClass == THOST_FTDC_PC_Options || instr.ProductClass == THOST_FTDC_PC_SpotOption;
            case THOST_FTDC_INS_COMB:
                return instr.ProductClass == THOST_FTDC_PC_Combination;
            default:
                return false;
            }
        };
        if (!matchClassType())
        {
            continue;
        }
        if (COMPARE_MEMBER_MATCH(pQryClassifiedInstrument, instr, ExchangeID) &&
            COMPARE_MEMBER_MATCH(pQryClassifiedInstrument, instr, ProductID) &&
            COMPARE_MEMBER_MATCH(pQryClassifiedInstrument, instr, InstrumentID))
        {
            v.emplace_back(&instr);
        }
    }
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        m_pSpi->OnRspQryClassifiedInstrument(*it, &m_successRspInfo, nRequestID, (it + 1 == v.end() ? true : false));
    }
    if (v.empty())
    {
        m_pSpi->OnRspQryClassifiedInstrument(nullptr, &m_successRspInfo, nRequestID, true);
    }
    return 0;
}

///请求组合优惠比例
int CLocalTraderApi::ReqQryCombPromotionParam(CThostFtdcQryCombPromotionParamField *pQryCombPromotionParam, int nRequestID) { return -1; }
