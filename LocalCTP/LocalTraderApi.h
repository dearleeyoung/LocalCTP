#pragma once

#include "stdafx.h"
#include "ThostFtdcTraderApi.h"//CTP交易的头文件
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

    ///注册用户终端信息，用于中继服务器多连接模式
    ///需要在终端认证成功后，用户登录前调用该接口
    virtual int RegisterUserSystemInfo(CThostFtdcUserSystemInfoField *pUserSystemInfo) override;

    ///上报用户终端信息，用于中继服务器操作员登录模式
    ///操作员登录后，可以多次调用该接口上报客户信息
    virtual int SubmitUserSystemInfo(CThostFtdcUserSystemInfoField *pUserSystemInfo) override;

    ///用户登录请求
    virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) override;

    ///登出请求
    virtual int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID) override;

    ///用户口令更新请求
    virtual int ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) override;

    ///资金账户口令更新请求
    virtual int ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) override;

    ///查询用户当前支持的认证模式
    virtual int ReqUserAuthMethod(CThostFtdcReqUserAuthMethodField *pReqUserAuthMethod, int nRequestID) override;

    ///用户发出获取图形验证码请求
    virtual int ReqGenUserCaptcha(CThostFtdcReqGenUserCaptchaField *pReqGenUserCaptcha, int nRequestID) override;

    ///用户发出获取短信验证码请求
    virtual int ReqGenUserText(CThostFtdcReqGenUserTextField *pReqGenUserText, int nRequestID) override;

    ///用户发出带有图片验证码的登陆请求
    virtual int ReqUserLoginWithCaptcha(CThostFtdcReqUserLoginWithCaptchaField *pReqUserLoginWithCaptcha, int nRequestID) override;

    ///用户发出带有短信验证码的登陆请求
    virtual int ReqUserLoginWithText(CThostFtdcReqUserLoginWithTextField *pReqUserLoginWithText, int nRequestID) override;

    ///用户发出带有动态口令的登陆请求
    virtual int ReqUserLoginWithOTP(CThostFtdcReqUserLoginWithOTPField *pReqUserLoginWithOTP, int nRequestID) override;

    ///报单录入请求
    virtual int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID) override;

    ///预埋单录入请求
    virtual int ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID) override;

    ///预埋撤单录入请求
    virtual int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID) override;

    ///报单操作请求
    virtual int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID) override;

    ///查询最大报单数量请求
    virtual int ReqQryMaxOrderVolume(CThostFtdcQryMaxOrderVolumeField *pQryMaxOrderVolume, int nRequestID) override;

    ///投资者结算结果确认
    virtual int ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) override;

    ///请求删除预埋单
    virtual int ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) override;

    ///请求删除预埋撤单
    virtual int ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) override;

    ///执行宣告录入请求
    virtual int ReqExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, int nRequestID) override;

    ///执行宣告操作请求
    virtual int ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, int nRequestID) override;

    ///询价录入请求
    virtual int ReqForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, int nRequestID) override;

    ///报价录入请求
    virtual int ReqQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, int nRequestID) override;

    ///报价操作请求
    virtual int ReqQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, int nRequestID) override;

    ///批量报单操作请求
    virtual int ReqBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, int nRequestID) override;

    ///期权自对冲录入请求
    virtual int ReqOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField *pInputOptionSelfClose, int nRequestID) override;

    ///期权自对冲操作请求
    virtual int ReqOptionSelfCloseAction(CThostFtdcInputOptionSelfCloseActionField *pInputOptionSelfCloseAction, int nRequestID) override;

    ///申请组合录入请求
    virtual int ReqCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, int nRequestID) override;

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

    ///请求查询交易编码
    virtual int ReqQryTradingCode(CThostFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) override;

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

    ///请求查询转帐银行
    virtual int ReqQryTransferBank(CThostFtdcQryTransferBankField *pQryTransferBank, int nRequestID) override;

    ///请求查询投资者持仓明细
    virtual int ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) override;

    ///请求查询客户通知
    virtual int ReqQryNotice(CThostFtdcQryNoticeField *pQryNotice, int nRequestID) override;

    ///请求查询结算信息确认
    virtual int ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) override;

    ///请求查询投资者持仓明细
    virtual int ReqQryInvestorPositionCombineDetail(CThostFtdcQryInvestorPositionCombineDetailField *pQryInvestorPositionCombineDetail, int nRequestID) override;

    ///请求查询保证金监管系统经纪公司资金账户密钥
    virtual int ReqQryCFMMCTradingAccountKey(CThostFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID) override;

    ///请求查询仓单折抵信息
    virtual int ReqQryEWarrantOffset(CThostFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID) override;

    ///请求查询投资者品种/跨品种保证金
    virtual int ReqQryInvestorProductGroupMargin(CThostFtdcQryInvestorProductGroupMarginField *pQryInvestorProductGroupMargin, int nRequestID) override;

    ///请求查询交易所保证金率
    virtual int ReqQryExchangeMarginRate(CThostFtdcQryExchangeMarginRateField *pQryExchangeMarginRate, int nRequestID) override;

    ///请求查询交易所调整保证金率
    virtual int ReqQryExchangeMarginRateAdjust(CThostFtdcQryExchangeMarginRateAdjustField *pQryExchangeMarginRateAdjust, int nRequestID) override;

    ///请求查询汇率
    virtual int ReqQryExchangeRate(CThostFtdcQryExchangeRateField *pQryExchangeRate, int nRequestID) override;

    ///请求查询二级代理操作员银期权限
    virtual int ReqQrySecAgentACIDMap(CThostFtdcQrySecAgentACIDMapField *pQrySecAgentACIDMap, int nRequestID) override;

    ///请求查询产品报价汇率
    virtual int ReqQryProductExchRate(CThostFtdcQryProductExchRateField *pQryProductExchRate, int nRequestID) override;

    ///请求查询产品组
    virtual int ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID) override;

    ///请求查询做市商合约手续费率
    virtual int ReqQryMMInstrumentCommissionRate(CThostFtdcQryMMInstrumentCommissionRateField *pQryMMInstrumentCommissionRate, int nRequestID) override;

    ///请求查询做市商期权合约手续费
    virtual int ReqQryMMOptionInstrCommRate(CThostFtdcQryMMOptionInstrCommRateField *pQryMMOptionInstrCommRate, int nRequestID) override;

    ///请求查询报单手续费
    virtual int ReqQryInstrumentOrderCommRate(CThostFtdcQryInstrumentOrderCommRateField *pQryInstrumentOrderCommRate, int nRequestID) override;

    ///请求查询资金账户
    virtual int ReqQrySecAgentTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) override;

    ///请求查询二级代理商资金校验模式
    virtual int ReqQrySecAgentCheckMode(CThostFtdcQrySecAgentCheckModeField *pQrySecAgentCheckMode, int nRequestID) override;

    ///请求查询二级代理商信息
    virtual int ReqQrySecAgentTradeInfo(CThostFtdcQrySecAgentTradeInfoField *pQrySecAgentTradeInfo, int nRequestID) override;

    ///请求查询期权交易成本
    virtual int ReqQryOptionInstrTradeCost(CThostFtdcQryOptionInstrTradeCostField *pQryOptionInstrTradeCost, int nRequestID) override;

    ///请求查询期权合约手续费
    virtual int ReqQryOptionInstrCommRate(CThostFtdcQryOptionInstrCommRateField *pQryOptionInstrCommRate, int nRequestID) override;

    ///请求查询执行宣告
    virtual int ReqQryExecOrder(CThostFtdcQryExecOrderField *pQryExecOrder, int nRequestID) override;

    ///请求查询询价
    virtual int ReqQryForQuote(CThostFtdcQryForQuoteField *pQryForQuote, int nRequestID) override;

    ///请求查询报价
    virtual int ReqQryQuote(CThostFtdcQryQuoteField *pQryQuote, int nRequestID) override;

    ///请求查询期权自对冲
    virtual int ReqQryOptionSelfClose(CThostFtdcQryOptionSelfCloseField *pQryOptionSelfClose, int nRequestID) override;

    ///请求查询投资单元
    virtual int ReqQryInvestUnit(CThostFtdcQryInvestUnitField *pQryInvestUnit, int nRequestID) override;

    ///请求查询组合合约安全系数
    virtual int ReqQryCombInstrumentGuard(CThostFtdcQryCombInstrumentGuardField *pQryCombInstrumentGuard, int nRequestID) override;

    ///请求查询申请组合
    virtual int ReqQryCombAction(CThostFtdcQryCombActionField *pQryCombAction, int nRequestID) override;

    ///请求查询转帐流水
    virtual int ReqQryTransferSerial(CThostFtdcQryTransferSerialField *pQryTransferSerial, int nRequestID) override;

    ///请求查询银期签约关系
    virtual int ReqQryAccountregister(CThostFtdcQryAccountregisterField *pQryAccountregister, int nRequestID) override;

    ///请求查询签约银行
    virtual int ReqQryContractBank(CThostFtdcQryContractBankField *pQryContractBank, int nRequestID) override;

    ///请求查询预埋单
    virtual int ReqQryParkedOrder(CThostFtdcQryParkedOrderField *pQryParkedOrder, int nRequestID) override;

    ///请求查询预埋撤单
    virtual int ReqQryParkedOrderAction(CThostFtdcQryParkedOrderActionField *pQryParkedOrderAction, int nRequestID) override;

    ///请求查询交易通知
    virtual int ReqQryTradingNotice(CThostFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID) override;

    ///请求查询经纪公司交易参数
    virtual int ReqQryBrokerTradingParams(CThostFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID) override;

    ///请求查询经纪公司交易算法
    virtual int ReqQryBrokerTradingAlgos(CThostFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID) override;

    ///请求查询监控中心用户令牌
    virtual int ReqQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, int nRequestID) override;

    ///期货发起银行资金转期货请求
    virtual int ReqFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) override;

    ///期货发起期货资金转银行请求
    virtual int ReqFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) override;

    ///期货发起查询银行余额请求
    virtual int ReqQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, int nRequestID) override;

    ///请求查询分类合约
    virtual int ReqQryClassifiedInstrument(CThostFtdcQryClassifiedInstrumentField *pQryClassifiedInstrument, int nRequestID) override;

    ///请求组合优惠比例
    virtual int ReqQryCombPromotionParam(CThostFtdcQryCombPromotionParamField *pQryCombPromotionParam, int nRequestID) override;
};
