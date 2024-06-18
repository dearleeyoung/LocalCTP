// TestLocalCTP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 本文件是测试 LocalCTP 的 DEMO, 仅用于展示 LocalCTP 的部分功能。

#include <iostream>
#include <cstring>
#include <string>
#include <fstream> // std::ofstream
#include "ThostFtdcTraderApi.h"//CTP交易的头文件
CThostFtdcInputOrderField generateNewOrderMsg(const char* OrderRef, const char* InstrumentID = "MA401")
{
    CThostFtdcInputOrderField InputOrder = { 0 };
    strcpy(InputOrder.UserID, "TestUserID");
    strcpy(InputOrder.InvestorID, "TestUserID");
    strcpy(InputOrder.AccountID, "TestUserID");
    strcpy(InputOrder.BrokerID, "9876");
    strcpy(InputOrder.ExchangeID, "CZCE");
    strcpy(InputOrder.InstrumentID, InstrumentID);
    strcpy(InputOrder.OrderRef, OrderRef);
    InputOrder.VolumeTotalOriginal = 1;
    InputOrder.LimitPrice = 5000.0;
    InputOrder.Direction = THOST_FTDC_D_Buy;
    InputOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    InputOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    InputOrder.VolumeCondition = THOST_FTDC_VC_AV;
    InputOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
    InputOrder.TimeCondition = THOST_FTDC_TC_GFD;//若为 THOST_FTDC_TC_IOC + THOST_FTDC_VC_CV 则为IOC订单
    return InputOrder;
}

CThostFtdcInputOrderActionField generateCancelOrderMsg(const char* OrderRef, const char* InstrumentID = "MA401",
    int frontID = 0, int sessionID = 0)
{
    CThostFtdcInputOrderActionField InputOrderAction = { 0 };
    strcpy(InputOrderAction.UserID, "TestUserID");
    strcpy(InputOrderAction.InvestorID, "TestUserID");
    strcpy(InputOrderAction.BrokerID, "9876");
    strcpy(InputOrderAction.ExchangeID, "CZCE");
    strcpy(InputOrderAction.InstrumentID, InstrumentID);
    strcpy(InputOrderAction.OrderRef, OrderRef);
    InputOrderAction.FrontID = frontID;
    InputOrderAction.SessionID = sessionID;
    InputOrderAction.ActionFlag = THOST_FTDC_AF_Delete;
    return InputOrderAction;
}

CThostFtdcTraderApi* pApi = nullptr;
int g_frontID = 0;
int g_sessionID = 0;

class MySpi : public CThostFtdcTraderSpi
{
    void OnFrontConnected(){
        CThostFtdcReqAuthenticateField ReqAuthenticateField = { "9876", "TestUserID", "Test", "TestAuthCode", "TestAppID" };

        pApi->ReqAuthenticate(&ReqAuthenticateField, 100);
    }
    void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到认证响应! " << " UserID:" << pRspAuthenticateField->UserID
            << " errorID:"<< pRspInfo->ErrorID<<" errorMsg:"<< pRspInfo->ErrorMsg << std::endl;
    }
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到登录响应! " << " UserID:" << pRspUserLogin->UserID
            << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << " TradingDay:" << pRspUserLogin->TradingDay << std::endl;
        g_frontID = pRspUserLogin->FrontID;
        g_sessionID = pRspUserLogin->SessionID;
    }
    void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询合约响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pInstrument)
        {
            std::cout << "InstrumentID:" << pInstrument->InstrumentID
                << ", ExchangeID:" << pInstrument->ExchangeID
                << ", InstrumentName:" << pInstrument->InstrumentName
                << ", PriceTick:" << pInstrument->PriceTick
                << ", VolumeMultiple:" << pInstrument->VolumeMultiple
                << std::endl;
        }
    }
    void OnRspQryClassifiedInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询分类合约响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pInstrument)
        {
            std::cout << "InstrumentID:" << pInstrument->InstrumentID
                << ", ExchangeID:" << pInstrument->ExchangeID
                << ", InstrumentName:" << pInstrument->InstrumentName
                << ", PriceTick:" << pInstrument->PriceTick
                << ", VolumeMultiple:" << pInstrument->VolumeMultiple
                << std::endl;
        }
    }
    void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询报单响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pOrder)
        {
            std::cout << "InstrumentID:" << pOrder->InstrumentID
                << ", ExchangeID:" << pOrder->ExchangeID
                << ", OrderSysID:" << pOrder->OrderSysID
                << ", OrderStatus:" << pOrder->OrderStatus
                << std::endl;
        }
    }
    void OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询成交响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pTrade)
        {
            std::cout << "InstrumentID:" << pTrade->InstrumentID
                << ", ExchangeID:" << pTrade->ExchangeID
                << ", OrderSysID:" << pTrade->OrderSysID
                << ", TradeID:" << pTrade->TradeID
                << std::endl;
        }
    }
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询持仓响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pInvestorPosition)
        {
            std::cout << "InstrumentID:" << pInvestorPosition->InstrumentID
                << ", ExchangeID:" << pInvestorPosition->ExchangeID
                << ", PosiDirection:" << (pInvestorPosition->PosiDirection== THOST_FTDC_PD_Long ? "多头":"空头")
                << ", PositionDate:" << pInvestorPosition->PositionDate
                << ", Position:" << pInvestorPosition->Position
                << std::endl;
        }
    }
    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "收到查询结算单响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
            << ", bIsLast:" << bIsLast << std::endl;
        if (pSettlementInfo)
        {
            //将结算单保存到本地文件中,文件名包含 账户名 和 结算单的交易日
            static std::ofstream o(std::string("./") + pSettlementInfo->AccountID
                + "_" + pSettlementInfo->TradingDay + "_settlement.log");
            o << pSettlementInfo->Content;//Content末尾可能正好有中文字符被截断(即分别在两个不同的消息里返回), 这里没有做处理
            o.flush();

            std::cout << "结算单 交易日:" << pSettlementInfo->TradingDay
                << ", 消息正文:" << pSettlementInfo->Content
                << std::endl;
            
        }
    }
    void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "\n收到(有错误的)报单响应! " << " UserID:" << pInputOrder->UserID
            << " InstrumentID:" << pInputOrder->InstrumentID
            << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
    }
    void OnRtnOrder(CThostFtdcOrderField* pOrder) {
        std::cout << "\n收到报单通知! " << " UserID:" << pOrder->UserID
            << " InstrumentID:" << pOrder->InstrumentID
            << " OrderSysID:" << pOrder->OrderSysID
            << " OrderStatus:" << pOrder->OrderStatus << " StatusMsg:" << pOrder->StatusMsg << std::endl;
    }
    void OnRtnTrade(CThostFtdcTradeField* pTrade) {
        std::cout << "\n收到成交通知! " << " UserID:" << pTrade->UserID
            << " InstrumentID:" << pTrade->InstrumentID
            << " Direction:" << (pTrade->Direction == THOST_FTDC_D_Buy ? "买入":"卖出")
            << " OffsetFlag:" << (pTrade->OffsetFlag == THOST_FTDC_OF_Open ? "开仓" : "平仓")
            << " Volume:" << pTrade->Volume << " Price:" << pTrade->Price << std::endl;
    }
    void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
        std::cout << "\n收到(有错误的)撤单响应! " << " UserID:" << pInputOrderAction->UserID
            << " InstrumentID:" << pInputOrderAction->InstrumentID
            << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
    }

};


int main()
{
    pApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    std::cout << pApi->GetApiVersion() << std::endl;
    //std::cout << pApi->GetTradingDay() << std::endl;
    MySpi spi;
    pApi->RegisterSpi(&spi);
    pApi->Init();

    std::cout << "请输入任意数字以登录..." << std::endl;
    int i;
    std::cin >> i;
    CThostFtdcReqUserLoginField ReqUserLoginField = { "", "9876", "TestUserID", "TestPassword" };
    pApi->ReqUserLogin(&ReqUserLoginField, 100);
    CThostFtdcQrySettlementInfoField QrySettlementInfo = { "9876", "TestUserID" };
    pApi->ReqQrySettlementInfo(&QrySettlementInfo, 0);//查询结算单
    CThostFtdcSettlementInfoConfirmField confirm = { "9876", "TestUserID" };
    pApi->ReqSettlementInfoConfirm(&confirm, 0);//确认结算单

    CThostFtdcQryInstrumentField QryInstrument = { 0 };
    strcpy(QryInstrument.ProductID, "IC");
    pApi->ReqQryInstrument(&QryInstrument, 108);
#if 0
    CThostFtdcQryClassifiedInstrumentField QryClassifiedInstrument = { 0 };
    strcpy(QryClassifiedInstrument.ProductID, "IF");
    QryClassifiedInstrument.ClassType = THOST_FTDC_INS_ALL;
    pApi->ReqQryClassifiedInstrument(&QryClassifiedInstrument, 108);
#endif

    CThostFtdcQryProductField QryProduct = { 0 };
    strcpy(QryProduct.ProductID, "jd");
    QryProduct.ProductClass = THOST_FTDC_PC_Futures;
    pApi->ReqQryProduct(&QryProduct, 108);

    // 查询完毕, 开始下单
    auto InputOrder = generateNewOrderMsg("1000", "MA403");
    int ret = pApi->ReqOrderInsert(&InputOrder,109);//没有行情数据时,下一单(预期被拒单)

    CThostFtdcDepthMarketDataField md = { 0 };
    //报入组合合约的委托, 并不需要有组合合约的行情, 而只需要有两个单腿合约的行情
    //strcpy(md.InstrumentID, "SPD MA309&MA401");
    //md.BidPrice1 = 4900;
    //md.AskPrice1 = 5000;
    //md.PreSettlementPrice = 6000;
    //pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API
    strcpy(md.InstrumentID, "MA403");
    md.BidPrice1 = 1000;
    md.AskPrice1 = 1010;
    md.SettlementPrice = md.PreSettlementPrice = 1020;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API
    strcpy(md.InstrumentID, "MA405");
    md.BidPrice1 = 2000;
    md.AskPrice1 = 2010;
    md.SettlementPrice = md.PreSettlementPrice = 2020;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API

    InputOrder = generateNewOrderMsg("1000", "SPD MA403&MA405");
    InputOrder.LimitPrice = -985;//实际会以1010-2000 = (-990)元的差价成交
    InputOrder.IsSwapOrder = 1;//互换单
    ret = pApi->ReqOrderInsert(&InputOrder, 109);//有行情数据后,再下一单(买入开仓成交)

    strcpy(InputOrder.OrderRef, "1001");
    InputOrder.Direction = THOST_FTDC_D_Sell;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    //    InputOrder.LimitPrice = -1111;//卖出时价格足够低则会成交
    ret = pApi->ReqOrderInsert(&InputOrder, 109);//有行情数据后,再下一单(卖出平仓)

    auto InputOrderAction = generateCancelOrderMsg("1001", "SPD MA403&MA405", g_frontID, g_sessionID);
    ret = pApi->ReqOrderAction(&InputOrderAction, 110);//把这个单子撤掉

    InputOrder = generateNewOrderMsg("1002", "MA403");
    InputOrder.Direction = THOST_FTDC_D_Buy;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    InputOrder.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;//下一个条件单
    InputOrder.StopPrice = 4998;
    ret = pApi->ReqOrderInsert(&InputOrder, 109);

    CThostFtdcInputQuoteField marketData = { 0 };
    strcpy(marketData.InstrumentID, "MA403");
    marketData.BidPrice = 1000;
    marketData.AskPrice = 1010;
    strcpy(marketData.QuoteRef, "1020.0");// PreSettlementPrice
    strcpy(marketData.UserID, "4999");// LastPrice.   4999 > 4998
    strcpy(marketData.ForQuoteSysID, "4999");// SettlementPrice
    strcpy(marketData.BidOrderRef, "9999.0");// UpperLimitPrice
    strcpy(marketData.AskOrderRef, "500.0");// LowerLimitPrice
    strcpy(marketData.BusinessUnit, "8888888");// OpenInterest
    int Volume = 88888;
    pApi->ReqQuoteInsert(&marketData, Volume);//使用ReqQuoteInsert接口, 喂一个行情快照给API(以触发条件单), 

    strcpy(md.InstrumentID, "IO2406-C-3300");
    md.BidPrice1 = 10.0;
    md.AskPrice1 = 10.4;
    md.SettlementPrice = md.PreSettlementPrice = 9.0;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个(期权)行情快照给API
    InputOrder = generateNewOrderMsg("1004", "IO2406-C-3300");
    strcpy(InputOrder.ExchangeID, "CFFEX");
    InputOrder.LimitPrice = 10.4;
    InputOrder.VolumeTotalOriginal = 2;
    ret = pApi->ReqOrderInsert(&InputOrder, 110);//有行情数据后,再下一单(买入开仓2手)
    strcpy(InputOrder.OrderRef, "1005");
    InputOrder.Direction = THOST_FTDC_D_Sell;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    InputOrder.LimitPrice = 10.0;
    InputOrder.VolumeTotalOriginal = 1;
    ret = pApi->ReqOrderInsert(&InputOrder, 111);//有行情数据后,再下一单(卖出平仓1手)

    CThostFtdcQryOrderField QryOrder = { "9876","TestUserID" };
    strcpy(QryOrder.ExchangeID, "CZCE");// 有数条CZCE的委托记录,查询返回
    ret = pApi->ReqQryOrder(&QryOrder, 100);

    CThostFtdcQryTradeField QryTrade = { "9876","TestUserID" };
    strcpy(QryTrade.ExchangeID, "DCE");// 没有DCE的成交记录,查询返回空
    ret = pApi->ReqQryTrade(&QryTrade, 100);

    CThostFtdcQryTradingAccountField QryTradingAccount = { "9876","TestUserID" };
    ret = pApi->ReqQryTradingAccount(&QryTradingAccount, 100);

    CThostFtdcQryInvestorPositionField QryInvestorPosition = { "9876","TestUserID" };
    strcpy(QryTrade.ExchangeID, "CZCE");
    ret = pApi->ReqQryInvestorPosition(&QryInvestorPosition, 100);

    std::cout << ret << std::endl;
    pApi->Release();
    std::cin >> i;
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
