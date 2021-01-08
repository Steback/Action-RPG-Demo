#ifndef PROTOTYPE_ACTION_RPG_LOGGER_HPP
#define PROTOTYPE_ACTION_RPG_LOGGER_HPP


#include <string>

#include "spdlog/spdlog.h"


namespace core {

    enum LogType{
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    class Logger {
    public:
        Logger();

        ~Logger();

        void init(const std::string& loggerName, const std::string& fileName);

        void sendLog(LogType type, const std::string& message);

    private:
        std::shared_ptr<spdlog::logger> mLogger;
        std::string mDirLogs;
    };

} // End namespace Core


#endif //PROTOTYPE_ACTION_RPG_LOGGER_HPP
