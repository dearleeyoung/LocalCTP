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
    HANDLE xinhao; //����ص������������ģ����Կ�
private:
    // ָ��CThostFtdcMduserApiʵ����ָ��
    CThostFtdcMdApi* m_pUserMdApi;    
    CThostFtdcTraderApi* m_pUserTdApi;
};
