#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace logger_mod {

Logger::Logger() {}

Logger::~Logger() {
    stop();
}

bool Logger::init_logger(const std::string& log_file_path) {
    std::lock_guard<std::mutex> lock(mtx_);

    if (log_file_.is_open()) {
        log_file_.flush();
        log_file_.close();
    }
    file_enable_ = false;

    if (log_file_path.empty()) {
        file_enable_ = false;
        return true;
    }

    log_file_.open(log_file_path, std::ios::out | std::ios::app);
    if (log_file_.is_open()) {
        file_enable_ = true;
        return true;
    }

    std::cerr << "[LOG ERROR]: ERROR PATH:" << log_file_path << std::endl;
    file_enable_ = false;
    return false;
}

void Logger::stop() {
    std::lock_guard<std::mutex> lock(mtx_);
    stop_flag_ = true;

    if (log_file_.is_open()) {
        log_file_.flush();
        log_file_.close();
    }
    file_enable_ = false;
}

void Logger::reset() {
    std::lock_guard<std::mutex> lock(mtx_);
    stop_flag_ = false;
}

void Logger::logger_send(LoggerLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (stop_flag_) {
        return;
    }

    std::string time_str = get_time_str();
    std::string level_str = level_to_str(level);
    std::string outline = "[" + time_str + "][" + level_str + "] " + msg;

    std::cout << outline << '\n';

    if (file_enable_ && log_file_.is_open()) {
        log_file_ << outline << '\n';
    }
}

std::string Logger::get_time_str() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::level_to_str(LoggerLevel level) {
    switch (level) {
    case LoggerLevel::INFO:
        return "[INFO]";
    case LoggerLevel::WARN:
        return "[WARN]";
    case LoggerLevel::ERROR:
        return "[ERROR]";
    default:
        return "[UNKNOWN]";
    }
}

}

