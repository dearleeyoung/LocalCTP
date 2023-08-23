#pragma once

#include "stdafx.h"
#include "ThostFtdcTraderApi.h"//CTP交易的头文件
#include "CTPApiHugeMacro.h"
#include "ctpStatus.h"
#include "CSqliteHandler.h"
#include "CTPSQLWrapper.h"

std::istream& operator>>(std::istream& i, CThostFtdcInstrumentField& instr);

class CLocalTraderApi :
    public CThostFtdcTraderApi,
    public std::enable_shared_from_this<CLocalTraderApi>
{
public:
    using SP_TRADE_API = std::shared_ptr<CLocalTraderApi>;
    using InstrMap = std::map<std::string, CThostFtdcInstrumentField>;
    using TradePriceVec = std::vector<double>;
    using MarketDataVec = std::vector<CThostFtdcDepthMarketDataField>;
    using MarketDataMap = std::map<std::string, CThostFtdcDepthMarketDataField>;

private:
    struct PositionData
    {
        PositionData() : volumeMultiple(1), pos{ 0 } { }
        int volumeMultiple;
        CThostFtdcInvestorPositionField pos;
        std::vector<CThostFtdcInvestorPositionDetailField> posDetailData;

        static CThostFtdcInvestorPositionDetailField getPositionDetailFromOpenTrade(
            const CThostFtdcTradeField& trade);
        // 比较两个持仓明细
        static bool comparePosDetail(const CThostFtdcInvestorPositionDetailField& lhs,
            const CThostFtdcInvestorPositionDetailField& rhs)
        {
            if (strcmp(lhs.OpenDate, rhs.OpenDate) < 0)
                return true;
            else if (strcmp(lhs.OpenDate, rhs.OpenDate) > 0)
                return false;
            else // 同一开仓日期的按开仓成交编号比较大小
                return strcmp(lhs.TradeID, rhs.TradeID) < 0;
        }
        // 对持仓明细排序
        void sortPositionDetail()
        {
            std::sort(posDetailData.begin(), posDetailData.end(), comparePosDetail);
        }
        // 插入或更新持仓明细
        void addPositionDetail(const CThostFtdcInvestorPositionDetailField& posDetail);
    };

    using PositionDataMap = std::map<std::string, PositionData>;

    struct OrderData
    {
        OrderData(CLocalTraderApi* pApi, const CThostFtdcInputOrderField& _inputOrder,
            bool _isConditionalOrder = false,
            const std::string& relativeOrderSysID = std::string())
            : inputOrder(_inputOrder), rtnOrder{ 0 }
            , api(*pApi), isConditionalOrder(_isConditionalOrder)
        {
            dealTestReqOrderInsertNormal(inputOrder, relativeOrderSysID);
        }
        bool isDone() const;
        void handleTrade(const TradePriceVec& tradedPriceVec, int tradedSize);
        void handleCancel(bool cancelFromClient = true);
        void sendRtnOrder();

        CThostFtdcInputOrderField inputOrder;
        CThostFtdcOrderField rtnOrder;
        std::vector<CThostFtdcTradeField> rtnTrades;

    private:
        void dealTestReqOrderInsertNormal(const CThostFtdcInputOrderField& InputOrder,
            const std::string& relativeOrderSysID);
        void sendRtnTrade(CThostFtdcTradeField& rtnTrade);
        void getRtnTrade(const TradePriceVec& tradePriceVec,
            int tradedSize, std::vector<CThostFtdcTradeField>& Trades);

        CLocalTraderApi& api;
        bool isConditionalOrder;
    };

public:
    // 储存交易API智能指针的集合
    static std::set<SP_TRADE_API> trade_api_set;
    static std::atomic<int> maxSessionID;
    static CSqliteHandler sqlHandler;

    // 判断是否成交
    static bool isMatchTrade(TThostFtdcDirectionType direction, double orderPrice,
        const MarketDataVec& mdVec, TradePriceVec& tradePriceVec);
    // 将组合合约代码拆分为单腿合约的数组. 支持处理多于2个单腿合约的组合合约.
    // input: CombinationContractID: 组合合约代码
    // input & output: SingleContracts: 拆分得到的单腿合约的数组
    // Example: "SPD MA309&MA401" -> {"MA309", "MA401"}
    // Example: "SPC a2401&m2401&y2401" -> {"a2401", "m2401", "y2401"}
    static void GetSingleContractFromCombinationContract(
        const std::string& CombinationContractID, std::vector<std::string>& SingleContracts);
    // 是否是明确区分今仓和昨仓的特殊交易所
    static bool isSpecialExchange(const std::string& exchangeID)
    {
        return (exchangeID == "SHFE" || exchangeID == "INE");
    }
    static TThostFtdcDirectionType getOppositeDirection(TThostFtdcDirectionType dir)
    {
        return (dir == THOST_FTDC_D_Buy ? THOST_FTDC_D_Sell : THOST_FTDC_D_Buy);
    }
    static TThostFtdcPosiDirectionType getPositionDirectionFromDirection(TThostFtdcDirectionType dir)
    {
        return (dir == THOST_FTDC_D_Buy ? THOST_FTDC_PD_Long : THOST_FTDC_PD_Short);
    }
    static std::string generatePositionKey(const CThostFtdcInvestorPositionField& pos)
    {
        return generatePositionKey(pos.InstrumentID,
            getPositionDirectionFromDirection(pos.PosiDirection),
            pos.PositionDate);
    }
    static std::string generatePositionKey(const std::string& instrumentID,
        TThostFtdcDirectionType dir, TThostFtdcPositionDateType dateType)
    {
        return instrumentID + "_" + dir + "_" + dateType;
    }
    static bool isOpen(TThostFtdcOffsetFlagType	OffsetFlag)
    {
        return OffsetFlag == THOST_FTDC_OF_Open;
    }
    static TThostFtdcPositionDateType getDateTypeFromOffset(const std::string& exchangeID,
        TThostFtdcOffsetFlagType offsetFlag)
    {
        if (offsetFlag == THOST_FTDC_OF_Open)
            return THOST_FTDC_PSD_Today;
        else if (isSpecialExchange(exchangeID))
        {
            if (offsetFlag == THOST_FTDC_OF_CloseToday)
                return THOST_FTDC_PSD_Today;
            else
                return THOST_FTDC_PSD_History;
        }
        else
            return THOST_FTDC_PSD_Today;
    }

    CLocalTraderApi(const char* pszFlowPath = "");
    ~CLocalTraderApi();
    CThostFtdcTraderSpi* getSpi() const { return m_pSpi; }
    std::atomic<int>& getOrderSysID() { return m_orderSysID; }
    std::atomic<int>& getTradeID() { return m_tradeID; }
    int getSessionID() const { return m_sessionID; }

private:
	std::atomic<bool> m_bRunning;
    std::atomic<bool> m_authenticated;
    std::atomic<bool> m_logined;
    std::atomic<int> m_orderSysID;
    std::atomic<int> m_tradeID;
    std::string m_userID;
    std::string m_brokerID;
    int m_sessionID;
    InstrMap m_instrData; //合约数据
    std::mutex m_mdMtx;

    MarketDataMap m_mdData; //行情数据
    std::mutex m_orderMtx;
    std::map<int, OrderData> m_orderData; //订单数据. key:OrderRef(报单引用).
    std::mutex m_positionMtx;
    PositionDataMap m_positionData; //持仓数据. key通过generatePositionKey生成.
    std::map<std::string, CThostFtdcInstrumentMarginRateField> m_instrumentMarginRateData;// 合约保证金数据. key:合约代码.
    std::map<std::string, CThostFtdcInstrumentCommissionRateField> m_instrumentCommissionRateData;// 合约手续费数据. key:合约代码.
    CThostFtdcTradingAccountField m_tradingAccount;// 资金数据
    std::map<std::string, OrderData> m_contionalOrders;// 条件报单数据. key:条件单报单编号(OrderSysID)
    std::map<std::string, CThostFtdcExchangeField> m_exchanges;// 交易所数据. key:交易所代码
    std::map<std::string, CThostFtdcProductField> m_products;// 品种数据. key:品种代码
	CThostFtdcTraderSpi* m_pSpi;//回调接口类的指针

    CThostFtdcRspInfoField m_successRspInfo;
    CThostFtdcRspInfoField m_errorRspInfo;
    CThostFtdcRspInfoField* setErrorMsgAndGetRspInfo(const char* errorMsg = "error");
    void onSnapshot(const CThostFtdcDepthMarketDataField& mdData);
    void updatePNL(bool needTotalCalc = false);
    void updateByCancel(const CThostFtdcOrderField& o);
    void updateByTrade(const CThostFtdcTradeField& t);
    void reloadAccountData();
    void saveTradingAccountToDb();
    void savePositionToDb(const PositionData& pos);
    void savePositionToDb(const CThostFtdcInvestorPositionField& pos);
    void savePositionDetialToDb(const CThostFtdcInvestorPositionDetailField& pos);
    void saveOrderToDb(const CThostFtdcOrderField& order);
    void saveTradeToDb(const CThostFtdcTradeField& trade);
    int ReqOrderInsertImpl(CThostFtdcInputOrderField* pInputOrder, int nRequestID,
        std::string relativeOrderSysID = std::string());

public:

    ///删除接口对象本身
    ///@remark 不再使用本接口对象时,调用该函数删除接口对象
    virtual void Release() override;

    ///初始化
    ///@remark 初始化运行环境,只有调用后,接口才开始工作
    virtual void Init() override;

    ///等待接口线程结束运行
    ///@return 线程退出代码
    virtual int Join() override;

    ///获取当前交易日
    ///@retrun 获取到的交易日
    ///@remark 只有登录成功后,才能得到正确的交易日
    virtual const char *GetTradingDay() override;

    ///注册前置机网络地址
    ///@param pszFrontAddress：前置机网络地址。
    ///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。 
    ///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
    virtual void RegisterFront(char *pszFrontAddress) override;

    ///注册名字服务器网络地址
    ///@param pszNsAddress：名字服务器网络地址。
    ///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:12001”。 
    ///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”12001”代表服务器端口号。
    ///@remark RegisterNameServer优先于RegisterFront
    virtual void RegisterNameServer(char *pszNsAddress) override;

    ///注册名字服务器用户信息
    ///@param pFensUserInfo：用户信息。
    virtual void RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo) override;

    ///注册回调接口
    ///@param pSpi 派生自回调接口类的实例
    virtual void RegisterSpi(CThostFtdcTraderSpi *pSpi) override;

    ///订阅私有流。
    ///@param nResumeType 私有流重传方式  
    ///        THOST_TERT_RESTART:从本交易日开始重传
    ///        THOST_TERT_RESUME:从上次收到的续传
    ///        THOST_TERT_QUICK:只传送登录后私有流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
    virtual void SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType) override;

    ///订阅公共流。
    ///@param nResumeType 公共流重传方式  
    ///        THOST_TERT_RESTART:从本交易日开始重传
    ///        THOST_TERT_RESUME:从上次收到的续传
    ///        THOST_TERT_QUICK:只传送登录后公共流的内容
    ///        THOST_TERT_NONE:取消订阅公共流
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
    virtual void SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType) override;

    ///客户端认证请求
    virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID) override;

    ///用户登录请求
    virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) override;

    ///登出请求
    virtual int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID) override;

    ///用户口令更新请求
    virtual int ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) override;

    ///资金账户口令更新请求
    virtual int ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) override;

    ///报单录入请求
    virtual int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID) override;

    ///预埋单录入请求
    virtual int ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID) override;

    ///预埋撤单录入请求
    virtual int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID) override;

    ///报单操作请求
    virtual int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID) override;

    ///查询最大报单数量请求.
    // 旧版函数名是 ReqQueryMaxOrderVolume. 为避免混淆,设置为默认不支持此接口
    // virtual int ReqQryMaxOrderVolume(CThostFtdcQryMaxOrderVolumeField *pQryMaxOrderVolume, int nRequestID) override;

    ///投资者结算结果确认
    virtual int ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) override;

    ///请求删除预埋单
    virtual int ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) override;

    ///请求删除预埋撤单
    virtual int ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) override;

    ///请求查询报单
    virtual int ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID) override;

    ///请求查询成交
    virtual int ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID) override;

    ///请求查询投资者持仓
    virtual int ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) override;

    ///请求查询资金账户
    virtual int ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) override;

    ///请求查询投资者
    virtual int ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID) override;

    ///请求查询合约保证金率
    virtual int ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID) override;

    ///请求查询合约手续费率
    virtual int ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) override;

    ///请求查询交易所
    virtual int ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID) override;

    ///请求查询产品
    virtual int ReqQryProduct(CThostFtdcQryProductField *pQryProduct, int nRequestID) override;

    ///请求查询合约
    virtual int ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID) override;

    ///请求查询行情
    virtual int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) override;

    ///请求查询投资者结算结果
    virtual int ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) override;

    ///请求查询投资者持仓明细
    virtual int ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) override;

    ///请求查询结算信息确认
    virtual int ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) override;

    ///请求查询分类合约
    // 这是自v6.5.1版本加入的函数.如果使用低版本API头文件则可以把这个接口注释掉
    virtual int ReqQryClassifiedInstrument(CThostFtdcQryClassifiedInstrumentField *pQryClassifiedInstrument, int nRequestID) override;

    // 以下是本系统不支持的CTP API接口
    UNSUPPORTED_CTP_API_FUNC
};
