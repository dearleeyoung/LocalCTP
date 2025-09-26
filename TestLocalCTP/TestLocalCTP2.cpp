// TestLocalCTP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 本文件是测试 LocalCTP 的 DEMO, 仅用于展示 LocalCTP 的部分功能。

#include "../LocalCTP/MarketDataPublisher.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h" //CTP交易的头文件
#include <cstring>
#include <fstream> // std::ofstream
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

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
    InputOrder.TimeCondition = THOST_FTDC_TC_GFD; // 若为 THOST_FTDC_TC_IOC + THOST_FTDC_VC_CV 则为IOC订单
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

CThostFtdcMdApi* mdapi = nullptr;

class MyMdSpi : public CThostFtdcMdSpi {
private:
    std::unique_ptr<MarketDataPublisher> publisher_;
    CThostFtdcMdApi* pUserApi = nullptr;

public:
    MyMdSpi(const char* mq_name = "md_queue", CThostFtdcMdApi* pUserApi = nullptr)
    {
        publisher_.reset(MarketDataPublisher::Create(mq_name, 1024, sizeof(CThostFtdcDepthMarketDataField)));
        if (!publisher_) {
            std::cerr << "Failed to create MarketDataPublisher" << std::endl;
        }
        this->pUserApi = pUserApi;
        std::cout << "MyMdSpi initialized" << std::endl;
    }

    void OnFrontConnected() override
    {
        std::cout << "收到连接成功通知!" << std::endl;
        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));

        snprintf(req.BrokerID, sizeof(req.BrokerID), "9999");
        snprintf(req.UserID, sizeof(req.UserID), "02071");

        int ret = pUserApi->ReqUserLogin(&req, 0);
        std::cout << "ReqUserLogin ret: " << ret << std::endl;
    }

    void OnFrontDisconnected(int nReason) override
    {
        std::cout << "收到连接断开通知! " << " nReason:" << nReason << std::endl;
    }

    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        if (pRspInfo && pRspInfo->ErrorID != 0) {
            std::cout << "收到行情登录响应! " << " UserID:" << pRspUserLogin->UserID
                      << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                      << " TradingDay:" << pRspUserLogin->TradingDay << std::endl;
            return;
        }

        std::cout << "收到行情登录响应! " << " UserID:" << pRspUserLogin->UserID
                  << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << " TradingDay:" << pRspUserLogin->TradingDay << std::endl;

        char* instrumentID[] = { "ag2512", "au2512" };
        pUserApi->SubscribeMarketData(instrumentID, 2);
    }

    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到订阅行情响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
    }

    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) override
    {
        if (pDepthMarketData) {
            std::cout << "收到行情推送! InstrumentID:" << pDepthMarketData->InstrumentID
                      << ", LastPrice:" << pDepthMarketData->LastPrice
                      << ", Volume:" << pDepthMarketData->Volume << std::endl;

            // 通过MarketDataPublisher发布行情数据
            if (publisher_) {
                bool success = publisher_->publish(pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
                if (!success) {
                    std::cerr << "Failed to publish market data for " << pDepthMarketData->InstrumentID << std::endl;
                } else {
                    std::cout << "Successfully published market data for " << pDepthMarketData->InstrumentID << std::endl;
                }
            }
        }
    }
};

class MySpi : public CThostFtdcTraderSpi {
    void OnFrontConnected()
    {
        std::cout << "收到连接成功通知!" << std::endl;
        CThostFtdcReqAuthenticateField ReqAuthenticateField = { "9876", "TestUserID", "Test", "TestAuthCode", "TestAppID" };
        pApi->ReqAuthenticate(&ReqAuthenticateField, 100);
    }
    void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到认证响应! " << " UserID:" << pRspAuthenticateField->UserID
                  << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
    }
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到交易端登录响应! " << " UserID:" << pRspUserLogin->UserID
                  << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << " TradingDay:" << pRspUserLogin->TradingDay << std::endl;
        g_frontID = pRspUserLogin->FrontID;
        g_sessionID = pRspUserLogin->SessionID;
    }
    void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询合约响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pInstrument) {
            std::cout << "InstrumentID:" << pInstrument->InstrumentID
                      << ", ExchangeID:" << pInstrument->ExchangeID
                      << ", InstrumentName:" << pInstrument->InstrumentName
                      << ", PriceTick:" << pInstrument->PriceTick
                      << ", VolumeMultiple:" << pInstrument->VolumeMultiple
                      << std::endl;
        }
    }
    void OnRspQryClassifiedInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询分类合约响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pInstrument) {
            std::cout << "InstrumentID:" << pInstrument->InstrumentID
                      << ", ExchangeID:" << pInstrument->ExchangeID
                      << ", InstrumentName:" << pInstrument->InstrumentName
                      << ", PriceTick:" << pInstrument->PriceTick
                      << ", VolumeMultiple:" << pInstrument->VolumeMultiple
                      << std::endl;
        }
    }
    void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询报单响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pOrder) {
            std::cout << "InstrumentID:" << pOrder->InstrumentID
                      << ", ExchangeID:" << pOrder->ExchangeID
                      << ", OrderSysID:" << pOrder->OrderSysID
                      << ", OrderStatus:" << pOrder->OrderStatus
                      << std::endl;
        }
    }
    void OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询成交响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pTrade) {
            std::cout << "InstrumentID:" << pTrade->InstrumentID
                      << ", ExchangeID:" << pTrade->ExchangeID
                      << ", OrderSysID:" << pTrade->OrderSysID
                      << ", TradeID:" << pTrade->TradeID
                      << std::endl;
        }
    }
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询持仓响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pInvestorPosition) {
            std::cout << "InstrumentID:" << pInvestorPosition->InstrumentID
                      << ", ExchangeID:" << pInvestorPosition->ExchangeID
                      << ", PosiDirection:" << (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long ? "多头" : "空头")
                      << ", PositionDate:" << pInvestorPosition->PositionDate
                      << ", Position:" << pInvestorPosition->Position
                      << std::endl;
        }
    }
    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "收到查询结算单响应! " << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg
                  << ", bIsLast:" << bIsLast << std::endl;
        if (pSettlementInfo) {
            // 将结算单保存到本地文件中,文件名包含 账户名 和 结算单的交易日
            static std::ofstream o(std::string("./") + pSettlementInfo->AccountID
                + "_" + pSettlementInfo->TradingDay + "_settlement.log");
            o << pSettlementInfo->Content; // Content末尾可能正好有中文字符被截断(即分别在两个不同的消息里返回), 这里没有做处理
            o.flush();

            std::cout << "结算单 交易日:" << pSettlementInfo->TradingDay
                      << ", 消息正文:" << pSettlementInfo->Content
                      << std::endl;
        }
    }
    void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "\n收到(有错误的)报单响应! " << " UserID:" << pInputOrder->UserID
                  << " InstrumentID:" << pInputOrder->InstrumentID
                  << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
    }
    void OnRtnOrder(CThostFtdcOrderField* pOrder)
    {
        std::cout << "\n收到报单通知! " << " UserID:" << pOrder->UserID
                  << " InstrumentID:" << pOrder->InstrumentID
                  << " OrderSysID:" << pOrder->OrderSysID
                  << " OrderStatus:" << pOrder->OrderStatus << " StatusMsg:" << pOrder->StatusMsg << std::endl;
    }
    void OnRtnTrade(CThostFtdcTradeField* pTrade)
    {
        std::cout << "\n收到成交通知! " << " UserID:" << pTrade->UserID
                  << " InstrumentID:" << pTrade->InstrumentID
                  << " Direction:" << (pTrade->Direction == THOST_FTDC_D_Buy ? "买入" : "卖出")
                  << " OffsetFlag:" << (pTrade->OffsetFlag == THOST_FTDC_OF_Open ? "开仓" : "平仓")
                  << " Volume:" << pTrade->Volume << " Price:" << pTrade->Price << std::endl;
    }
    void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
    {
        std::cout << "\n收到(有错误的)撤单响应! " << " UserID:" << pInputOrderAction->UserID
                  << " InstrumentID:" << pInputOrderAction->InstrumentID
                  << " errorID:" << pRspInfo->ErrorID << " errorMsg:" << pRspInfo->ErrorMsg << std::endl;
    }
};

int sample1()
{
    int i;

    pApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    std::cout << "TraderApi Version: [" << pApi->GetApiVersion() << "]" << std::endl;
    // std::cout << "TraderApi TradingDay: [" << pApi->GetTradingDay() << "]" << std::endl;
    MySpi spi;
    pApi->RegisterSpi(&spi);
    pApi->Init();

    mdapi = CThostFtdcMdApi::CreateFtdcMdApi();
    std::cout << mdapi->GetApiVersion() << std::endl;

    // 创建MyMdSpi实例
    MyMdSpi mdSpi("md_queue", mdapi);

    mdapi->RegisterFront("tcp://182.254.243.31:40011");
    // mdapi->RegisterFront("tcp://182.254.243.31:30011");
    mdapi->RegisterSpi(&mdSpi);
    mdapi->Init();

    std::cout << "请输入任意数字以登录..." << std::endl;
    std::cin >> i;
    CThostFtdcReqUserLoginField ReqUserLoginField = { "", "9876", "TestUserID", "TestPassword" };
    pApi->ReqUserLogin(&ReqUserLoginField, 100);
    CThostFtdcQrySettlementInfoField QrySettlementInfo = { "9876", "TestUserID" };
    pApi->ReqQrySettlementInfo(&QrySettlementInfo, 0); // 查询结算单
    CThostFtdcSettlementInfoConfirmField confirm = { "9876", "TestUserID" };
    pApi->ReqSettlementInfoConfirm(&confirm, 0); // 确认结算单

    CThostFtdcQryInstrumentField QryInstrument = {};
    strcpy(QryInstrument.ProductID, "au");
    int ret = pApi->ReqQryInstrument(&QryInstrument, 108);

    std::cin >> i;
    std::cout << ret << std::endl;
    pApi->Release();
    std::cin >> i;
    return 0;
}

int main()
{
    sample1();
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
