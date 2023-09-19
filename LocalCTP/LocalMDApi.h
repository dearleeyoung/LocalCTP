#pragma once

#include <ThostFtdcMdApi.h>
#include <ThostFtdcTraderApi.h>
#include <memory>




class semaphore
{
public:
    semaphore(int value = 1) :count(value) {}

    void wait()
    {
        std::unique_lock<std::mutex> lck(mt);
        if (--count < 0)//资源不足挂起线程
            cv.wait(lck);
    }

    void signal()
    {
        std::unique_lock<std::mutex> lck(mt);
        if (++count <= 0)//有线程挂起，唤醒一个
            cv.notify_one();
    }

private:
    int count;
    std::mutex mt;
    std::condition_variable cv;
};

class CLocalMdSpi : public CThostFtdcMdSpi {
public:
    CLocalMdSpi(CThostFtdcMdApi* pUserMdApi, CThostFtdcTraderApi* pUserTdApi);
    void OnFrontConnected();
    void ReqUserLogin();
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
        CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
    //HANDLE xinhao; //行情回调锁定，不懂的，试试看
    semaphore xinhao;
private:
    // 指向CThostFtdcMduserApi实例的指针
    CThostFtdcMdApi* m_pUserMdApi;    
    CThostFtdcTraderApi* m_pUserTdApi;
};
