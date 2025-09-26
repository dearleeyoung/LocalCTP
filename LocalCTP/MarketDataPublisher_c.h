#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// 不透明指针类型，隐藏 C++ 实现细节
typedef struct MarketDataPublisher_t MarketDataPublisher_t;

/**
 * 创建 MarketDataPublisher 实例
 * @param mq_name 消息队列名称，如果为 NULL 则使用默认值 "md_mq"
 * @param max_msg 最大消息数量，如果为 0 则使用默认值 1024
 * @param msg_size 消息大小，如果为 0 则使用默认值 1024
 * @return MarketDataPublisher 实例指针，失败返回 NULL
 */
MarketDataPublisher_t* marketdata_publisher_create(const char* mq_name,
    unsigned int max_msg,
    unsigned int msg_size);

/**
 * 销毁 MarketDataPublisher 实例
 * @param publisher 要销毁的实例指针
 */
void marketdata_publisher_destroy(MarketDataPublisher_t* publisher);

/**
 * 发布市场数据
 * @param publisher MarketDataPublisher 实例指针
 * @param data 要发布的数据指针
 * @param size 数据大小
 * @return true 发布成功，false 发布失败
 */
bool marketdata_publisher_publish(MarketDataPublisher_t* publisher,
    const void* data,
    size_t size);

#ifdef __cplusplus
}
#endif
