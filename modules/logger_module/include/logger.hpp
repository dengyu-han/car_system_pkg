#pragma once
#include <string>
#include <mutex>
#include <fstream>

namespace logger_mod {

enum class LoggerLevel {
    INFO,
    WARN,
    ERROR
};

class Logger {
private:
    mutable std::mutex mtx_;
    std::ofstream log_file_;
    bool stop_flag_{false};
    bool file_enable_{false};

private:
    std::string get_time_str();
    std::string level_to_str(LoggerLevel level);

public:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    bool init_logger(const std::string& log_file_path);
    void stop();
    void reset();
    void logger_send(LoggerLevel level, const std::string& msg);
};

}

