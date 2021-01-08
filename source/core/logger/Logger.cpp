#include "Logger.hpp"

#include <filesystem>

#include "spdlog/sinks/basic_file_sink.h"


namespace core {

    Logger::Logger() : mDirLogs("../logs") {

    }

    Logger::~Logger() = default;

    void Logger::init(const std::string& loggerName, const std::string& fileName) {
        if (!std::filesystem::exists(mDirLogs)) {
            std::filesystem::create_directory(mDirLogs);
        }

        auto fileName_ = mDirLogs + "/" + fileName;

        if (std::filesystem::exists(fileName_)) {
            std::filesystem::remove(fileName_);
        }

        try {
            mLogger = spdlog::basic_logger_mt(loggerName, fileName_);
        } catch (const spdlog::spdlog_ex& ex) {
            fmt::print(stderr, "{}\n", ex.what());
        }
    }

} // End namespace core
