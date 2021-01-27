#ifndef PROTOTYPE_ACTION_RPG_UTILITIES_HPP
#define PROTOTYPE_ACTION_RPG_UTILITIES_HPP

#include <string>
#include <stdexcept>


namespace core {

    inline void throw_ex(const std::string& message) {
        throw std::runtime_error(message);
    };

}

#endif //PROTOTYPE_ACTION_RPG_UTILITIES_HPP
