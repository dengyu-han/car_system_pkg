#include <gtest/gtest.h>
#include "logger.hpp"
#include <string>
#include <filesystem>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

TEST(LoggerTest, LogLevel)
{
    logger_mod::Logger log;
    log.init_logger("");
    log.logger_send(logger_mod::LoggerLevel::INFO, "[INFO] READY");
    log.logger_send(logger_mod::LoggerLevel::WARN, "[WARN] READY");
    log.logger_send(logger_mod::LoggerLevel::ERROR, "[ERROR] READY");
}

TEST(LoggerTest, InitOfEmpty)
{
    logger_mod::Logger log;
    bool ok = log.init_logger("");
    ASSERT_TRUE(ok);
    log.logger_send(logger_mod::LoggerLevel::INFO, "[INFO] READY");
}

TEST(LoggerTest, InitOfRealPath)
{
    std::string test_dir = "./logger";
    std::string test_log = "./logger/test.log";

    if (!fs::exists(test_dir))
    {
        fs::create_directory(test_dir);
    }
    
    if (fs::exists(test_log))
    {
        fs::remove(test_log);
    }

    logger_mod::Logger log;
    bool ok = log.init_logger(test_log);
    ASSERT_TRUE(ok);

    log.logger_send(logger_mod::LoggerLevel::INFO, "[INFO] READY");
    log.stop();

    ASSERT_TRUE(fs::exists(test_log));

    // 清理
    fs::remove(test_log);
    fs::remove(test_dir);
}

TEST(LoggerTest, InitOfBadPath)
{
    std::string bad_path = "./root/radar.log";
    logger_mod::Logger log;

    bool ok = log.init_logger(bad_path);
    ASSERT_FALSE(ok);

    log.logger_send(logger_mod::LoggerLevel::WARN, "[WARN] READY");
}

TEST(LoggerTest, StopAndReset)
{
    std::string test_dir = "./logger";
    std::string test_log = "./logger/test.log";

    if (!fs::exists(test_dir))
    {
        fs::create_directory(test_dir);
    }
    if (fs::exists(test_log))
    {
        fs::remove(test_log);
    }

    logger_mod::Logger log;
    bool ok = log.init_logger(test_log);
    ASSERT_TRUE(ok);

    log.logger_send(logger_mod::LoggerLevel::INFO, "SEND TO LOG");

    log.stop();
    log.logger_send(logger_mod::LoggerLevel::ERROR, "NO SEND");

    log.reset();
    ASSERT_TRUE(log.init_logger(""));
    log.logger_send(logger_mod::LoggerLevel::INFO, "SEND TO COUT");

    log.stop();
    fs::remove(test_log);
    fs::remove(test_dir);
}

// 多线程并发写日志
TEST(LoggerTest, MultiThreadWrite)
{
    logger_mod::Logger log;
    log.init_logger("");

    std::vector<std::thread> th_vec;
    const int thread_cnt = 8;
    const int msg_per_thread = 20;

    for(int i = 0; i < thread_cnt; ++i)
    {
        th_vec.emplace_back([&log, i, msg_per_thread](){
            for(int j = 0; j < msg_per_thread; j++)
            {
                std::string msg = "thread:" + std::to_string(i) + " msg:" + std::to_string(j);
                log.logger_send(logger_mod::LoggerLevel::INFO, msg);
            }
        });
    }

    for(auto& th : th_vec)
    {
        th.join();
    }
    log.stop();
}

