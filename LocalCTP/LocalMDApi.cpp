#include "stdafx.h"
#include "LocalMDApi.h"
#include "LocalTraderApi.h"


#define LOG(format, ...) printf(format, __VA_ARGS__)

CLocalMdSpi::CLocalMdSpi(CThostFtdcMdApi* pUserMdApi, CThostFtdcTraderApi* pUserTdApi)
    : m_pUserMdApi(pUserMdApi), m_pUserTdApi(pUserTdApi) {
    xinhao = CreateEvent(NULL, false, false, NULL);
};

void CLocalMdSpi::OnFrontConnected()
{
    /*strcpy_s(g_chBrokerID, getConfig("config", "BrokerID").c_str());
    strcpy_s(g_chUserID, getConfig("config", "UserID").c_str());
    strcpy_s(g_chPassword, getConfig("config", "Password").c_str());*/
    LOG("<OnFrontConnected>\n");
    LOG("api version is %s", m_pUserMdApi->GetApiVersion());    
    ReqUserLogin();
}

void CLocalMdSpi::ReqUserLogin()
{
    CThostFtdcReqUserLoginField reqUserLogin;
    int num = m_pUserMdApi->ReqUserLogin(&reqUserLogin, 0);
    LOG("\tlogin num = %d\n", num);

}

// ���ͻ��˷�����¼����֮�󣬸÷����ᱻ���ã�֪ͨ�ͻ��˵�¼�Ƿ�ɹ�
void CLocalMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    LOG("OnRspUserLogin:\n");
    LOG("\tErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,
        pRspInfo->ErrorMsg);
    LOG("\tRequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
        // �˵�ʧ�ܣ��ͻ�������д�����
        LOG("\tFailed to login, errorcode=%d errormsg=%s requestid=%d chain = %d",
            pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
        exit(-1);
    }

    // û�е�½�ɹ�֮ǰ���ܶ���
    SetEvent(xinhao);


}
///�������֪ͨ
void CLocalMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) {
    
    if (pDepthMarketData && false)
    {
        LOG("<OnRtnDepthMarketData>\n");
        if (strlen(pDepthMarketData->InstrumentID) == 0) {
            LOG("\treserve1 = [%s]\n", pDepthMarketData->reserve1);
            LOG("\treserve2 = [%s]\n", pDepthMarketData->reserve2);
        }
        else {
            LOG("\tInstrumentID = [%s]\n", pDepthMarketData->InstrumentID);
            LOG("\treserve1 = [%s]\n", pDepthMarketData->reserve1);
        }
        LOG("\tExchangeID = [%s]\n", pDepthMarketData->ExchangeID);
        LOG("\tLastPrice = [%.8lf]\n", pDepthMarketData->LastPrice);
        LOG("\tPreSettlementPrice = [%.8lf]\n", pDepthMarketData->PreSettlementPrice);
        LOG("\tOpenPrice = [%.8lf]\n", pDepthMarketData->OpenPrice);
        LOG("\tVolume = [%d]\n", pDepthMarketData->Volume);
        LOG("\tTurnover = [%.8lf]\n", pDepthMarketData->Turnover);
        LOG("\tOpenInterest = [%.8lf]\n", pDepthMarketData->OpenInterest);

        LOG("</OnRtnDepthMarketData>\n");
    }
    
    //�ؼ�������Ͷι
    m_pUserTdApi->RegisterFensUserInfo(reinterpret_cast<CThostFtdcFensUserInfoField*>(pDepthMarketData));
};
