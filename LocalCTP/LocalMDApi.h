#pragma once

#include <ThostFtdcMdApi.h>
#include <ThostFtdcTraderApi.h>
#include <memory>


class CLocalMdSpi : public CThostFtdcMdSpi {
public:
    CLocalMdSpi(CThostFtdcMdApi* pUserMdApi, CThostFtdcTraderApi* pUserTdApi);
    void OnFrontConnected();
    void ReqUserLogin();
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
        CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
    HANDLE xinhao; //行情回调锁定，不懂的，试试看
private:
    // 指向CThostFtdcMduserApi实例的指针
    CThostFtdcMdApi* m_pUserMdApi;    
    CThostFtdcTraderApi* m_pUserTdApi;
};
