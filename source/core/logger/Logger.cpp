#include "Logger.hpp"

#include <filesystem>

#include "spdlog/sinks/basic_file_sink.h"


namespace core {

    Logger::Logger() : dirLogs("../logs") {

    }

    Logger::~Logger() = default;

    void Logger::init(const std::string& loggerName, const std::string& fileName) {
        if (!std::filesystem::exists(dirLogs)) {
            std::filesystem::create_directory(dirLogs);
        }

        auto fileName_ = dirLogs + '/' + fileName;

        if (std::filesystem::exists(fileName_)) {
            std::filesystem::remove(fileName_);
        }

        try {
            m_logger = spdlog::basic_logger_mt(loggerName, fileName_);
        } catch (const spdlog::spdlog_ex& ex) {
            fmt::print(stderr, "{}\n", ex.what());
        }
    }

    void Logger::sendLog(LogType type, const std::string &message) {
        switch (type) {
            case INFO:
                m_logger->info(message);
                break;
            case WARNING:
                m_logger->warn(message);
                break;
            case ERROR:
                m_logger->error(message);
                break;
        }
    }

} // End namespace core
