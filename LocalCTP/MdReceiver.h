
#pragma once
#ifndef MD_RECEIVER_H
#define MD_RECEIVER_H

#include <atomic>
#include <memory>
#include <thread>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

// 需要完整类型以与交易API交互
#include "ThostFtdcUserApiStruct.h"

// 全局函数声明 - 打印市场数据结构体
void printDepthMarketData(const CThostFtdcDepthMarketDataField* pDepthMarketData);

namespace localCTP {

class CLocalTraderApi;

class MdReceiver {
public:
    MdReceiver(const char* pszFlowPath, CLocalTraderApi* api);
    ~MdReceiver();

    void start();
    void stop();

private:
    void threadFunc();

    std::unique_ptr<boost::interprocess::message_queue> mq_;
    CLocalTraderApi* traderApi_;
    std::atomic<bool> running_;
    std::thread worker_;
};
}
#endif