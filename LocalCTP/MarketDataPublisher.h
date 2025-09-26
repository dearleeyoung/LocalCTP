#pragma once
#include <memory>
#include <string>

class MarketDataPublisher {
public:
    virtual ~MarketDataPublisher() = default;

    static MarketDataPublisher* Create(const char* mq_name = "md_queue",
        unsigned int max_msg = 1024,
        unsigned int msg_size = 1024);

    virtual bool publish(const void* data, std::size_t size) = 0;
};
