#include "core/logger/Logger.hpp"

int main() {
    core::Logger logger;
    logger.init("logger", "error.log");

    return EXIT_SUCCESS;
}