//
// Created by wjf on 2023/12/9.
//

#ifndef FUTURES_TRADE_SDK2_ANDROID_LOGCAT_BUF_H
#define FUTURES_TRADE_SDK2_ANDROID_LOGCAT_BUF_H

#include <string>
#include <android/log.h>
#include <iostream>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AndroidLogcatBuf : public std::streambuf {
public:
    enum {
        bufsize = 128
    }; // ... or some other suitable buffer size

    AndroidLogcatBuf();

private:
    char buffer[bufsize];

    int overflow(int c) override;

    int sync() override;

public:
    static std::atomic<bool> m_redirect_stdout;

    /**
     * 重定向std::cout输出到 logcat
     */
    static void redirect_stdout();

    /**
     * 读取assets文件内容
     * AAssetManager *aAssetManager = AAssetManager_fromJava(env, assetManager);
     * @param assetManager 资源管理器
     * @param filename 文件名
     * @return 文件内容
     */
    static std::string readAssetFile(AAssetManager *assetManager, const std::string &filename);
};

#endif //FUTURES_TRADE_SDK2_ANDROID_LOGCAT_BUF_H
