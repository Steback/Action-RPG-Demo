#include "core/logger/Logger.hpp"

int main() {
    core::Logger logger;
    logger.init("logger", "error.logs");

    return EXIT_SUCCESS;
}