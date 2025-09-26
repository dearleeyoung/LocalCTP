#include <boost/interprocess/ipc/message_queue.hpp>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "MarketDataPublisher.h"
#include "MarketDataPublisher_c.h"
#include "ThostFtdcUserApiStruct.h"

using namespace boost::interprocess;

class MarketDataPublisherImpl : public MarketDataPublisher {
public:
    MarketDataPublisherImpl(const char* mq_name,
        unsigned int max_msg,
        unsigned int msg_size)
        : mq_name_(mq_name ? mq_name : "")
        , is_valid_(false)
    {
        try {
            // 参数验证
            if (!mq_name || strlen(mq_name) == 0) {
                throw std::invalid_argument("Message queue name cannot be null or empty");
            }

            if (max_msg == 0) {
                throw std::invalid_argument("Max message count cannot be zero");
            }

            if (msg_size == 0) {
                throw std::invalid_argument("Message size cannot be zero");
            }

            // 尝试打开已存在的消息队列，如果不存在则报错
            try {
                mq_ = std::make_unique<message_queue>(open_only, mq_name_.c_str());
                std::cout << "[INFO] Opened existing message queue: " << mq_name_ << std::endl;
            } catch (const interprocess_exception& ex) {
                if (ex.get_error_code() == not_found_error) {
                    std::cerr << "[ERROR] Message queue '" << mq_name_ << "' does not exist" << std::endl;
                    throw std::runtime_error("Message queue does not exist: " + mq_name_);
                } else {
                    std::cerr << "[ERROR] Failed to open message queue '" << mq_name_
                              << "': " << ex.what() << std::endl;
                    throw;
                }
            }

            is_valid_ = true;
            std::cout << "[INFO] MarketDataPublisher initialized successfully for queue: "
                      << mq_name_ << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] MarketDataPublisher initialization failed: "
                      << ex.what() << std::endl;
            is_valid_ = false;
            throw;
        } catch (...) {
            std::cerr << "[ERROR] MarketDataPublisher initialization failed: Unknown error" << std::endl;
            is_valid_ = false;
            throw;
        }
    }

    ~MarketDataPublisherImpl() override
    {
        try {
            if (mq_) {
                std::cout << "[INFO] Destroying MarketDataPublisher for queue: "
                          << mq_name_ << std::endl;
                mq_.reset();
                // 可选：清理消息队列
                // message_queue::remove(mq_name_.c_str());
            }
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] Error in MarketDataPublisher destructor: "
                      << ex.what() << std::endl;
        } catch (...) {
            std::cerr << "[ERROR] Unknown error in MarketDataPublisher destructor" << std::endl;
        }
    }

    bool publish(const void* data, std::size_t size) override
    {
        try {
            // 状态检查
            if (!is_valid_ || !mq_) {
                std::cerr << "[ERROR] MarketDataPublisher is not in valid state" << std::endl;
                return false;
            }

            // 参数验证
            if (!data) {
                std::cerr << "[ERROR] Cannot publish null data" << std::endl;
                return false;
            }

            if (size == 0) {
                std::cerr << "[ERROR] Cannot publish zero-sized data" << std::endl;
                return false;
            }

            // 尝试发送数据
            bool success = mq_->try_send(data, size, 0);
            if (!success) {
                std::cerr << "[WARNING] Message queue is full, failed to send "
                          << size << " bytes" << std::endl;
            }

            return success;
        } catch (const interprocess_exception& ex) {
            std::cerr << "[ERROR] Interprocess error while publishing: "
                      << ex.what() << std::endl;
            return false;
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] Error while publishing: " << ex.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "[ERROR] Unknown error while publishing" << std::endl;
            return false;
        }
    }

    // 新增：检查发布器状态
    bool is_valid() const
    {
        return is_valid_ && mq_ != nullptr;
    }

    // 新增：获取队列名称
    const std::string& get_queue_name() const
    {
        return mq_name_;
    }

private:
    std::string mq_name_;
    std::unique_ptr<message_queue> mq_;
    bool is_valid_;
};

MarketDataPublisher* MarketDataPublisher::Create(const char* mq_name,
    unsigned int max_msg,
    unsigned int msg_size)
{
    try {
        // 参数预验证
        if (!mq_name || strlen(mq_name) == 0) {
            std::cerr << "[ERROR] MarketDataPublisher::Create - Invalid queue name" << std::endl;
            return nullptr;
        }

        if (max_msg == 0) {
            std::cerr << "[ERROR] MarketDataPublisher::Create - Invalid max message count: "
                      << max_msg << std::endl;
            return nullptr;
        }

        if (msg_size == 0) {
            std::cerr << "[ERROR] MarketDataPublisher::Create - Invalid message size: "
                      << msg_size << std::endl;
            return nullptr;
        }

        std::cout << "[INFO] Creating MarketDataPublisher with queue='" << mq_name
                  << "', max_msg=" << max_msg << ", msg_size=" << msg_size << std::endl;

        return new MarketDataPublisherImpl(mq_name, max_msg, msg_size);
    } catch (const std::bad_alloc& ex) {
        std::cerr << "[ERROR] Memory allocation failed in MarketDataPublisher::Create: "
                  << ex.what() << std::endl;
        return nullptr;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception in MarketDataPublisher::Create: "
                  << ex.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception in MarketDataPublisher::Create" << std::endl;
        return nullptr;
    }
}

// C++ 类的包装结构体
struct MarketDataPublisher_t {
    MarketDataPublisher* cpp_instance;

    MarketDataPublisher_t(MarketDataPublisher* instance)
        : cpp_instance(instance)
    {
    }

    ~MarketDataPublisher_t()
    {
        delete cpp_instance;
    }
};

extern "C" {

MarketDataPublisher_t* marketdata_publisher_create(const char* mq_name,
    unsigned int max_msg,
    unsigned int msg_size)
{
    try {
        // 设置默认值
        const char* name = (mq_name != nullptr) ? mq_name : "md_queue";
        unsigned int max_messages = (max_msg > 0) ? max_msg : 1024;
        unsigned int message_size = (msg_size > 0) ? msg_size : 1024;

        std::cout << "[INFO] C API: Creating MarketDataPublisher with name='" << name
                  << "', max_msg=" << max_messages << ", msg_size=" << message_size << std::endl;

        // 创建 C++ 实例
        MarketDataPublisher* cpp_instance = MarketDataPublisher::Create(name, max_messages, message_size);
        if (cpp_instance == nullptr) {
            std::cerr << "[ERROR] C API: Failed to create MarketDataPublisher instance" << std::endl;
            return nullptr;
        }

        // 创建包装结构体
        MarketDataPublisher_t* wrapper = new MarketDataPublisher_t(cpp_instance);
        std::cout << "[INFO] C API: MarketDataPublisher wrapper created successfully" << std::endl;
        return wrapper;
    } catch (const std::bad_alloc& ex) {
        std::cerr << "[ERROR] C API: Memory allocation failed: " << ex.what() << std::endl;
        return nullptr;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] C API: Exception in marketdata_publisher_create: "
                  << ex.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "[ERROR] C API: Unknown exception in marketdata_publisher_create" << std::endl;
        return nullptr;
    }
}

void marketdata_publisher_destroy(MarketDataPublisher_t* publisher)
{
    try {
        if (publisher != nullptr) {
            std::cout << "[INFO] C API: Destroying MarketDataPublisher wrapper" << std::endl;
            delete publisher;
            std::cout << "[INFO] C API: MarketDataPublisher wrapper destroyed successfully" << std::endl;
        } else {
            std::cerr << "[WARNING] C API: Attempt to destroy null publisher pointer" << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] C API: Exception in marketdata_publisher_destroy: "
                  << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] C API: Unknown exception in marketdata_publisher_destroy" << std::endl;
    }
}

bool marketdata_publisher_publish(MarketDataPublisher_t* publisher,
    const void* data,
    size_t size)
{
    try {
        // 参数验证
        if (publisher == nullptr) {
            std::cerr << "[ERROR] C API: Publisher pointer is null" << std::endl;
            return false;
        }

        if (publisher->cpp_instance == nullptr) {
            std::cerr << "[ERROR] C API: Publisher C++ instance is null" << std::endl;
            return false;
        }

        if (data == nullptr) {
            std::cerr << "[ERROR] C API: Data pointer is null" << std::endl;
            return false;
        }

        if (size == 0) {
            std::cerr << "[ERROR] C API: Data size is zero" << std::endl;
            return false;
        }

        // 尝试发布数据
        bool result = publisher->cpp_instance->publish(data, size);
        // if (result) {
        //     std::cout << "[DEBUG] C API: Successfully published " << size << " bytes" << std::endl;
        // } else {
        //     std::cerr << "[WARNING] C API: Failed to publish " << size << " bytes" << std::endl;
        // }

        return result;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] C API: Exception in marketdata_publisher_publish: "
                  << ex.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "[ERROR] C API: Unknown exception in marketdata_publisher_publish" << std::endl;
        return false;
    }
}

} // extern "C"
