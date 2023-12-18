//
// Created by wjf on 2023/12/9.
//

#include "android_logcat_buf.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <iostream>
#include <string>

std::string AndroidLogcatBuf::readAssetFile(AAssetManager *assetManager, const std::string &filename) {
    AAsset *asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        return "";
    }

    off_t fileSize = AAsset_getLength(asset);
    std::string fileContent;
    fileContent.resize(fileSize);

    // 读取文件内容到字符串
    AAsset_read(asset, &fileContent[0], fileSize);

    AAsset_close(asset);

    return fileContent;
}

std::atomic<bool> AndroidLogcatBuf::m_redirect_stdout = {false};

void AndroidLogcatBuf::redirect_stdout() {
    auto hasNotSet = false;
    auto exchanged = m_redirect_stdout.compare_exchange_strong(hasNotSet, true, std::memory_order_seq_cst);
    if (!exchanged) {
        return;
    }
    auto buf = new AndroidLogcatBuf();
    std::cout.rdbuf(buf);
    std::cerr.rdbuf(buf);
    std::clog.rdbuf(buf);
    // avoid a one-time resource leak but don't get output afterwards:
//    delete std::cout.rdbuf(0);
}

AndroidLogcatBuf::AndroidLogcatBuf() {
    this->setp(buffer, buffer + bufsize - 1);
}

int AndroidLogcatBuf::overflow(int c) {
    if (c == traits_type::eof()) {
        *this->pptr() = traits_type::to_char_type(c);
        this->sbumpc();
    }
    return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
}

int AndroidLogcatBuf::sync() {
    int rc = 0;
    if (this->pbase() != this->pptr()) {
        __android_log_print(ANDROID_LOG_INFO,
                            "STDOUT_LOGCAT",
                            "%s",
                            std::string(this->pbase(),
                                        this->pptr() - this->pbase()).c_str());
        rc = 0;
        this->setp(buffer, buffer + bufsize - 1);
    }
    return rc;
}
