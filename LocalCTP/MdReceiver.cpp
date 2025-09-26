
#include <cfloat>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

#include <boost/interprocess/shared_memory_object.hpp>

#include "LocalTraderApi.h"
#include "MdReceiver.h"

using namespace boost::interprocess;

// 打印CThostFtdcDepthMarketDataField结构体的函数
void printDepthMarketData(const CThostFtdcDepthMarketDataField* pDepthMarketData)
{
    if (!pDepthMarketData) {
        std::cout << "DepthMarketData is NULL" << std::endl;
        return;
    }

    std::cout << "=== Market Data ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);

    // 基本信息
    std::cout << "TradingDay     : " << pDepthMarketData->TradingDay << std::endl;
    std::cout << "InstrumentID  : " << pDepthMarketData->InstrumentID << std::endl;
    std::cout << "ExchangeID    : " << pDepthMarketData->ExchangeID << std::endl;
    std::cout << "ExchangeInstID: " << pDepthMarketData->ExchangeInstID << std::endl;

    // 价格信息
    std::cout << "LastPrice     : " << pDepthMarketData->LastPrice << std::endl;
    std::cout << "PreSettlementPrice: " << pDepthMarketData->PreSettlementPrice << std::endl;
    std::cout << "PreClosePrice : " << pDepthMarketData->PreClosePrice << std::endl;
    std::cout << "PreOpenInterest: " << pDepthMarketData->PreOpenInterest << std::endl;
    std::cout << "OpenPrice     : " << pDepthMarketData->OpenPrice << std::endl;
    std::cout << "HighestPrice  : " << pDepthMarketData->HighestPrice << std::endl;
    std::cout << "LowestPrice   : " << pDepthMarketData->LowestPrice << std::endl;

    // 成交量和持仓量
    std::cout << "Volume        : " << pDepthMarketData->Volume << std::endl;
    std::cout << "Turnover      : " << pDepthMarketData->Turnover << std::endl;
    std::cout << "OpenInterest  : " << pDepthMarketData->OpenInterest << std::endl;
    std::cout << "ClosePrice    : " << pDepthMarketData->ClosePrice << std::endl;
    std::cout << "SettlementPrice: " << pDepthMarketData->SettlementPrice << std::endl;

    // 涨跌停价格
    std::cout << "UpperLimitPrice: " << pDepthMarketData->UpperLimitPrice << std::endl;
    std::cout << "LowerLimitPrice: " << pDepthMarketData->LowerLimitPrice << std::endl;

    // 时间信息
    std::cout << "UpdateTime    : " << pDepthMarketData->UpdateTime << std::endl;
    std::cout << "UpdateMillisec: " << pDepthMarketData->UpdateMillisec << std::endl;

    // 买盘信息
    std::cout << "BidPrice1     : " << pDepthMarketData->BidPrice1 << std::endl;
    std::cout << "BidVolume1    : " << pDepthMarketData->BidVolume1 << std::endl;
    std::cout << "AskPrice1     : " << pDepthMarketData->AskPrice1 << std::endl;
    std::cout << "AskVolume1    : " << pDepthMarketData->AskVolume1 << std::endl;

    // 五档买盘
    if (pDepthMarketData->BidPrice2 != DBL_MAX) {
        std::cout << "BidPrice2     : " << pDepthMarketData->BidPrice2 << " Volume: " << pDepthMarketData->BidVolume2 << std::endl;
        std::cout << "BidPrice3     : " << pDepthMarketData->BidPrice3 << " Volume: " << pDepthMarketData->BidVolume3 << std::endl;
        std::cout << "BidPrice4     : " << pDepthMarketData->BidPrice4 << " Volume: " << pDepthMarketData->BidVolume4 << std::endl;
        std::cout << "BidPrice5     : " << pDepthMarketData->BidPrice5 << " Volume: " << pDepthMarketData->BidVolume5 << std::endl;
    }

    // 五档卖盘
    if (pDepthMarketData->AskPrice2 != DBL_MAX) {
        std::cout << "AskPrice2     : " << pDepthMarketData->AskPrice2 << " Volume: " << pDepthMarketData->AskVolume2 << std::endl;
        std::cout << "AskPrice3     : " << pDepthMarketData->AskPrice3 << " Volume: " << pDepthMarketData->AskVolume3 << std::endl;
        std::cout << "AskPrice4     : " << pDepthMarketData->AskPrice4 << " Volume: " << pDepthMarketData->AskVolume4 << std::endl;
        std::cout << "AskPrice5     : " << pDepthMarketData->AskPrice5 << " Volume: " << pDepthMarketData->AskVolume5 << std::endl;
    }

    // 均价信息
    std::cout << "AveragePrice  : " << pDepthMarketData->AveragePrice << std::endl;

    std::cout << "===================" << std::endl;
}

namespace localCTP {

MdReceiver::MdReceiver(const char* pszFlowPath, CLocalTraderApi* api)
    : traderApi_(api)
    , running_(false)
{
    // 构建消息队列文件路径
    std::string flowPath(pszFlowPath ? pszFlowPath : ".");
    std::string queueFilePath = flowPath + "/md_queue";

    try {
        // 清理目录下的同名文件 (使用POSIX函数替代std::filesystem)
        struct stat buffer;
        if (stat(queueFilePath.c_str(), &buffer) == 0) {
            if (unlink(queueFilePath.c_str()) == 0) {
                std::cout << "Removed existing message queue file: " << queueFilePath << std::endl;
            }
        }

        // 尝试移除可能存在的消息队列
        message_queue::remove("md_queue");

        // 创建消息队列实例
        mq_ = std::make_unique<message_queue>(create_only, "md_queue", 1024, sizeof(CThostFtdcDepthMarketDataField));

        std::cout << "Message queue initialized at path: " << flowPath << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error initializing message queue: " << e.what() << std::endl;
        throw;
    }
}

MdReceiver::~MdReceiver()
{
    stop();

    // 清理消息队列
    try {
        message_queue::remove("md_queue");
    } catch (const std::exception& e) {
        // 静默处理清理错误，避免析构函数抛出异常
        std::cerr << "Warning: Error cleaning up message queue: " << e.what() << std::endl;
    }
}

void MdReceiver::start()
{
    running_ = true;
    worker_ = std::thread(&MdReceiver::threadFunc, this);
}

void MdReceiver::stop()
{
    running_ = false;
    if (worker_.joinable())
        worker_.join();
}

void MdReceiver::threadFunc()
{
    const std::chrono::milliseconds timeout(1000); // 500ms超时
    std::cout << "MdReceiver thread started" << std::endl;
    while (running_) {
        CThostFtdcDepthMarketDataField md;
        size_t recv_size = 0;
        unsigned int priority = 0;

        bool received = mq_->timed_receive(&md, sizeof(md), recv_size, priority,
            boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(timeout.count()));
        if (received && recv_size == sizeof(md)) {
            // std::cout << "Received market data message, size: " << recv_size << std::endl;

            // 打印完整的行情数据
            // printDepthMarketData(&md);

            // 调用trader API处理行情快照
            traderApi_->onSnapshot(md);

            // std::cout << "Received message size: " << recv_size << std::endl;
        } else {
            // 超时或其他原因，可做其他处理或直接continue等待
            // std::cout << "No message received in timeout period\n";
        }
    }
}

}
