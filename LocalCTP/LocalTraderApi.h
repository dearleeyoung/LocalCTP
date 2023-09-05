#pragma once

#include "stdafx.h"
#include "ThostFtdcTraderApi.h"//CTP交易的头文件
#include "CTPApiHugeMacro.h"
#include "ctpStatus.h"
#include "CSqliteHandler.h"
#include "CTPSQLWrapper.h"
#include <thread>

namespace localCTP {

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
        // 对持仓明细排序. 在持仓明细数据数量(笔数)发生变化后需调用.
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
            const std::string& relativeOrderSysID = std::string())
            : inputOrder(_inputOrder)
            , rtnOrder(OrderData::genrateRtnOrderFromInputOrder(inputOrder) )
            , api(*pApi)
            , isConditionalOrder(isConditionalType(_inputOrder.ContingentCondition))
        {
            dealTestReqOrderInsertNormal(inputOrder, relativeOrderSysID);
        }
        // 通过已有的报单回报来创建. 适用于用数据源中读取到报单回报然后创建OrderData.
        OrderData(CLocalTraderApi* pApi, const CThostFtdcOrderField& _rtnOrder)
            : inputOrder(OrderData::genrateInputOrderFromRtnOrder(_rtnOrder))
            , rtnOrder(_rtnOrder)
            , api(*pApi)
            , isConditionalOrder(_rtnOrder.ContingentCondition)
        {
        }
        bool isDone() const;
        void handleTrade(const TradePriceVec& tradedPriceVec, int tradedSize);
        void handleCancel(bool cancelFromClient = true);
        void sendRtnOrder();

        const CThostFtdcInputOrderField inputOrder;
        CThostFtdcOrderField rtnOrder;
        std::vector<localCTP::CThostFtdcTradeFieldWrapper> rtnTrades;

    private:
        static CThostFtdcInputOrderField genrateInputOrderFromRtnOrder(
            const CThostFtdcOrderField& o);
        static CThostFtdcOrderField genrateRtnOrderFromInputOrder(
            const CThostFtdcInputOrderField& inputO);
        void dealTestReqOrderInsertNormal(const CThostFtdcInputOrderField& InputOrder,
            const std::string& relativeOrderSysID);
        void sendRtnTrade(localCTP::CThostFtdcTradeFieldWrapper& rtnTrade);
        void getRtnTrade(const TradePriceVec& tradePriceVec,
            int tradedSize, std::vector<localCTP::CThostFtdcTradeFieldWrapper>& Trades);

        CLocalTraderApi& api;
        bool isConditionalOrder;
    };

    class CSettlementHandler
    {
        CSettlementHandler(CSqliteHandler& _sqlHandler);
        CSqliteHandler& m_sqlHandler;
        std::atomic<bool> m_running;
        const int m_sleepSecond;
        const int m_settlementStartHour;
        int m_count;
        std::thread m_timerThread;

        std::string format_settlement_header1;
        std::string format_settlement_header2;
        std::string format_settlement_header3;
        std::string format_settlement_header4;
        std::string format_settlement_header5;
        std::string format_settlement_header6;
        std::string format_settlement_header7;
        std::string format_settlement_header8;
        std::string format_settlement_header9;
        std::string format_settlement_header10;
        std::string format_settlement_account_summary1;
        std::string format_settlement_account_summary2;
        std::string format_settlement_account_summary3;
        std::string format_settlement_account_summary4;
        std::string format_settlement_account_summary5;
        std::string format_settlement_account_summary6;
        std::string format_settlement_account_summary7;
        std::string format_settlement_account_summary8;
        std::string format_settlement_account_summary9;
        std::string format_settlement_account_summary10;
        std::string format_settlement_account_summary11;
        std::string format_settlement_account_summary12;
        std::string format_settlement_account_summary13;
        std::string format_settlement_account_summary14;
        std::string format_settlement_account_summary15;
        std::string format_settlement_account_summary16;
        std::string format_settlement_deposit_withdrawal_head1;
        std::string format_settlement_deposit_withdrawal_head2;
        std::string format_settlement_deposit_withdrawal_head3;
        std::string format_settlement_deposit_withdrawal_head4;
        std::string format_settlement_deposit_withdrawal_head5;
        std::string format_settlement_deposit_withdrawal_single_record1;
        std::string format_settlement_deposit_withdrawal_end1;
        std::string format_settlement_deposit_withdrawal_end2;
        std::string format_settlement_deposit_withdrawal_end3;
        std::string format_settlement_deposit_withdrawal_end4;
        std::string format_settlement_deposit_withdrawal_end5;
        std::string format_settlement_trade_head1;
        std::string format_settlement_trade_head2;
        std::string format_settlement_trade_head3;
        std::string format_settlement_trade_head4;
        std::string format_settlement_trade_head5;
        std::string format_settlement_trade_single_record1;
        std::string format_settlement_trade_end1;
        std::string format_settlement_trade_end2;
        std::string format_settlement_trade_end3;
        std::string format_settlement_trade_end4;
        std::string format_settlement_trade_end5;
        std::string format_settlement_trade_end6;
        std::string format_settlement_trade_end7;
        std::string format_settlement_trade_end8;
        std::string format_settlement_position_closed_head1;
        std::string format_settlement_position_closed_head2;
        std::string format_settlement_position_closed_head3;
        std::string format_settlement_position_closed_head4;
        std::string format_settlement_position_closed_head5;
        std::string format_settlement_position_closed_single_record1;
        std::string format_settlement_position_closed_end1;
        std::string format_settlement_position_closed_end2;
        std::string format_settlement_position_closed_end3;
        std::string format_settlement_position_closed_end4;
        std::string format_settlement_position_closed_end5;
        std::string format_settlement_position_closed_end6;
        std::string format_settlement_position_detail_head1;
        std::string format_settlement_position_detail_head2;
        std::string format_settlement_position_detail_head3;
        std::string format_settlement_position_detail_head4;
        std::string format_settlement_position_detail_head5;
        std::string format_settlement_position_detail_single_record1;
        std::string format_settlement_position_detail_end1;
        std::string format_settlement_position_detail_end2;
        std::string format_settlement_position_detail_end3;
        std::string format_settlement_position_detail_end4;
        std::string format_settlement_position_detail_end5;
        std::string format_settlement_position_detail_end6;
        std::string format_settlement_position_detail_end7;
        std::string format_settlement_position_head1;
        std::string format_settlement_position_head2;
        std::string format_settlement_position_head3;
        std::string format_settlement_position_head4;
        std::string format_settlement_position_head5;
        std::string format_settlement_position_single_record1;
        std::string format_settlement_position_end1;
        std::string format_settlement_position_end2;
        std::string format_settlement_position_end3;
        std::string format_settlement_position_end4;
        std::string format_settlement_position_end5;
        std::string format_settlement_position_end6;
        std::string format_settlement_position_end7;
        std::string format_settlement_position_end8;

        void init_format_settlement();
        bool checkSettlement();//检查结算情况
        void doSettlement();//开始结算!
        void doUserSettlement(
            const CThostFtdcTradingAccountFieldWrapper& tradingAccountFieldWrapper,
            const std::string& TradingDay);//对单个用户的结算
        void doWorkAfterSettlement(const std::string& oldTradingDay,
            const std::string& newTradingDay);//处理结算后的工作
    public:
        //单实例模式
        static CSettlementHandler& getSettlementHandler(CSqliteHandler& _sqlHandler)
        {
            static CSettlementHandler h(_sqlHandler);
            return h;
        }
        ~CSettlementHandler();
    };
public:
    static std::set<SP_TRADE_API> trade_api_set;// 储存交易API智能指针的集合
    static std::atomic<int> maxSessionID; // 当前最大会话编号
    static std::map<std::string, long long> m_orderSysID; // 当前最大委托编号
    static std::map<std::string, long long> m_tradeID; // 当前最大成交编号
    static InstrMap m_instrData; //合约数据
    static std::map<std::string, CThostFtdcExchangeField> m_exchanges;// 交易所数据. key:交易所代码
    static std::map<std::string, CThostFtdcProductField> m_products;// 品种数据. key:品种代码
    static CSqliteHandler sqlHandler; // SQL管理器
    static CSettlementHandler& settlementHandler; // 结算管理器
    static std::mutex m_mdMtx; // 行情数据互斥体
    static MarketDataMap m_mdData; // 行情数据
    static const long long initStartTime;

    // 生成会话的key
    static std::string generateSessionKey(int frontID, int sessionID)
    {
        return std::to_string(frontID) + "_" + std::to_string(sessionID);
    }
    // 获取下一个最大的委托编号
    static long long getNextOrderSysID(const std::string& exchangeID) {
        auto it = m_orderSysID.find(exchangeID);
        if (it == m_orderSysID.end())
        {
            m_orderSysID[exchangeID] = initStartTime;
            return initStartTime;
        }
        else
            return ++(it->second);
    }
    // 获取下一个最大的成交编号
    static long long getNextTradeID(const std::string& exchangeID) {
        auto it = m_tradeID.find(exchangeID);
        if (it == m_tradeID.end())
        {
            m_tradeID[exchangeID] = initStartTime;
            return initStartTime;
        }
        else
            return ++(it->second);
    }
    // 判断是否成交条件.
    // input: mdVec. 订单有关的合约的行情数据的列表(若为组合合约需确保为组合的各个单腿的顺序).
    // input & output: tradePriceVec. 成交价格的列表(若为组合合约则为按组合的各个单腿的顺序).
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
    // 获取反方向的买卖方向
    static TThostFtdcDirectionType getOppositeDirection(TThostFtdcDirectionType dir)
    {
        return (dir == THOST_FTDC_D_Buy ? THOST_FTDC_D_Sell : THOST_FTDC_D_Buy);
    }
    // 通过买卖方向获取持仓多空方向
    static TThostFtdcPosiDirectionType getPositionDirectionFromDirection(TThostFtdcDirectionType dir)
    {
        return (dir == THOST_FTDC_D_Buy ? THOST_FTDC_PD_Long : THOST_FTDC_PD_Short);
    }
    // 通过持仓多空方向获取买卖方向
    static TThostFtdcDirectionType getDirectionFromPositionDirection(TThostFtdcPosiDirectionType dir)
    {
        return (dir == THOST_FTDC_PD_Long ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell);
    }
    // 生成持仓的key
    static std::string generatePositionKey(const CThostFtdcInvestorPositionField& pos)
    {
        return generatePositionKey(pos.InstrumentID,
            getDirectionFromPositionDirection(pos.PosiDirection),
            pos.PositionDate);
    }
    // 生成持仓的key
    static std::string generatePositionKey(const std::string& instrumentID,
        TThostFtdcDirectionType dir, TThostFtdcPositionDateType dateType)
    {
        return instrumentID + "_" + dir + "_" + dateType;
    }
    // 判断是否是开仓类型
    static bool isOpen(TThostFtdcOffsetFlagType	OffsetFlag)
    {
        return OffsetFlag == THOST_FTDC_OF_Open;
    }
    // 通过开平方向获取持仓日期类型
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
    static void initInstrMap();

    CLocalTraderApi(const char* pszFlowPath = "");
    ~CLocalTraderApi();
    CThostFtdcTraderSpi* getSpi() const { return m_pSpi; }
    int getFrontID() const { return m_frontID; }
    int getSessionID() const { return m_sessionID; }

private:
	std::atomic<bool> m_bRunning; // 是否在运行
    std::atomic<bool> m_authenticated; // 是否已认证
    std::atomic<bool> m_logined; // 是否已登录
    std::string m_userID;// 用户名
    std::string m_brokerID;// 期货公司代码
    const int m_frontID;// 本连接前置编号. 每次启动进程时值是不同的.
    int m_sessionID;// 本连接会话编号. 从0开始自增.

    std::mutex m_orderMtx;// 订单数据互斥体
    std::map<std::string, std::map<int, OrderData>> m_orderData;// 订单数据. outer key:FrontID+SessionID(generateSessionKey生成), inner key:OrderRef(报单引用).
    std::mutex m_positionMtx;// 持仓数据互斥体
    PositionDataMap m_positionData; // 持仓数据. key通过generatePositionKey生成.
    std::map<std::string, CThostFtdcInstrumentMarginRateField> m_instrumentMarginRateData;// 合约保证金数据. key:合约代码.
    std::map<std::string, CThostFtdcInstrumentCommissionRateField> m_instrumentCommissionRateData;// 合约手续费数据. key:合约代码.
    CThostFtdcTradingAccountField m_tradingAccount;// 资金数据
    std::vector<OrderData*> m_contionalOrders;// 条件报单数据
	CThostFtdcTraderSpi* m_pSpi;// 回调接口类的指针

    CThostFtdcRspInfoField m_successRspInfo;// 成功的响应信息
    CThostFtdcRspInfoField m_errorRspInfo;// 错误的响应信息
    CThostFtdcRspInfoField* setErrorMsgAndGetRspInfo(const char* errorMsg = "error");// 设置并返回错误的响应信息
    void onSnapshot(const CThostFtdcDepthMarketDataField& mdData);// 处理行情快照的接口
    void updatePNL(bool needTotalCalc = false);// 更新PNL(盈亏)的接口
    void updateByCancel(const CThostFtdcOrderField& o);// 根据已撤单的委托更新账户数据的接口,在委托被撤单后被调用
    void updateByTrade(const localCTP::CThostFtdcTradeFieldWrapper& t);// 根据成交更新账户数据的接口,在发生成交后被调用
    void reloadAccountData();// 从数据库中重新加载账户数据的接口
    void saveTradingAccountToDb();// 保存账户资金数据到数据库中的接口
    void savePositionToDb(const PositionData& pos);// 保存账户持仓数据到数据库中的接口
    void savePositionToDb(const CThostFtdcInvestorPositionField& pos);// 保存账户持仓数据到数据库中的接口
    void savePositionDetialToDb(const CThostFtdcInvestorPositionDetailField& pos);// 保存账户持仓明细数据到数据库中的接口
    void saveOrderToDb(const CThostFtdcOrderField& order);// 保存账户委托数据到数据库中的接口
    template<class T>
    void saveDataToDb(const T& wrapper)
    {
        const std::string sqlStr = wrapper.generateInsertSql();
        bool ret = sqlHandler.Insert(sqlStr);
        if (!ret)
        {
            // check?
        }
    }
    int ReqOrderInsertImpl(CThostFtdcInputOrderField* pInputOrder, int nRequestID,
        std::string relativeOrderSysID = std::string());// 处理新委托的实现的接口

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
    // 本接口被改造为接收行情快照数据的接口.
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

    ///报价录入请求
    // 本接口被改造为接收行情快照数据的接口.
    virtual int ReqQuoteInsert(CThostFtdcInputQuoteField* pInputQuote, int nRequestID) override;

    ///请求查询分类合约
    // 这是自v6.5.1版本加入的函数.如果使用低版本API头文件则可以把这个接口注释掉
    virtual int ReqQryClassifiedInstrument(CThostFtdcQryClassifiedInstrumentField *pQryClassifiedInstrument, int nRequestID) override;

    // 以下是本系统不支持的CTP API接口
    UNSUPPORTED_CTP_API_FUNC
};

} // end namespace localCTP
