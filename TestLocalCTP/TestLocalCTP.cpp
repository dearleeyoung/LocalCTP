// TestLocalCTP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 本文件是测试 LocalCTP 的 DEMO, 仅用于展示 LocalCTP 的部分功能。

#include <iostream>
#include <cstring>
#include <string>
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

CThostFtdcInputOrderActionField generateCancelOrderMsg(const char* OrderRef, const char* InstrumentID = "MA401")
{
    CThostFtdcInputOrderActionField InputOrderAction = { 0 };
    strcpy(InputOrderAction.UserID, "TestUserID");
    strcpy(InputOrderAction.InvestorID, "TestUserID");
    strcpy(InputOrderAction.BrokerID, "9876");
    strcpy(InputOrderAction.ExchangeID, "CZCE");
    strcpy(InputOrderAction.InstrumentID, InstrumentID);
    strcpy(InputOrderAction.OrderRef, OrderRef);
    InputOrderAction.FrontID = 0;
    InputOrderAction.SessionID = 0;
    InputOrderAction.ActionFlag = THOST_FTDC_AF_Delete;
    return InputOrderAction;
}

CThostFtdcTraderApi* pApi = nullptr;
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
            << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
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
    void  OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
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
    std::cout << "Hello World!" << std::endl;
    pApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    std::cout << pApi->GetApiVersion() << std::endl;
    std::cout << pApi->GetTradingDay() << std::endl;
    MySpi spi;
    pApi->RegisterSpi(&spi);
    pApi->Init();//读取 instrument.csv 合约文件
    
    CThostFtdcReqUserLoginField ReqUserLoginField = { "", "9876", "TestUserID", "TestPassword" };
    pApi->ReqUserLogin(&ReqUserLoginField, 100);

    CThostFtdcQryInstrumentField QryInstrument = { 0 };
    strcpy(QryInstrument.ProductID, "IC");
    pApi->ReqQryInstrument(&QryInstrument, 108);

    CThostFtdcQryProductField QryProduct = { 0 };
    strcpy(QryProduct.ProductID, "jd");
    QryProduct.ProductClass = THOST_FTDC_PC_Futures;
    pApi->ReqQryProduct(&QryProduct, 108);

    auto InputOrder = generateNewOrderMsg("1000", "SPD MA309&MA401");
    int ret = pApi->ReqOrderInsert(&InputOrder,109);//没有行情数据时,下一单(预期被拒单)

    CThostFtdcDepthMarketDataField md = { 0 };
    strcpy(md.InstrumentID, "SPD MA309&MA401");
    md.BidPrice1 = 4900;
    md.AskPrice1 = 5000;
    md.PreSettlementPrice = 6000;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API
    strcpy(md.InstrumentID, "MA309");
    md.BidPrice1 = 1000;
    md.AskPrice1 = 1010;
    md.PreSettlementPrice = 1020;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API
    strcpy(md.InstrumentID, "MA401");
    md.BidPrice1 = 2000;
    md.AskPrice1 = 2010;
    md.PreSettlementPrice = 2020;
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API
    InputOrder = generateNewOrderMsg("1000", "SPD MA309&MA401");
    ret = pApi->ReqOrderInsert(&InputOrder, 109);//有行情数据后,再下一单

    strcpy(InputOrder.OrderRef, "1001");
    InputOrder.Direction = THOST_FTDC_D_Sell;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    ret = pApi->ReqOrderInsert(&InputOrder, 109);//有行情数据后,再下一单(平仓)

    auto InputOrderAction = generateCancelOrderMsg("1001", "SPD MA309&MA401");
    ret = pApi->ReqOrderAction(&InputOrderAction, 110);//把这个单子撤掉

    InputOrder = generateNewOrderMsg("1002", "MA309");
    InputOrder.Direction = THOST_FTDC_D_Buy;
    InputOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    InputOrder.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;//下一个条件单
    InputOrder.StopPrice = 4998;
    ret = pApi->ReqOrderInsert(&InputOrder, 109);
    strcpy(md.InstrumentID, "MA309");
    md.BidPrice1 = 1000;
    md.AskPrice1 = 1010;
    md.PreSettlementPrice = 1020;
    md.LastPrice = 4997; // 4999 > 4998
    pApi->RegisterFensUserInfo((CThostFtdcFensUserInfoField*)&md);//喂一个行情快照给API(以触发条件单)

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
    int i;
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
