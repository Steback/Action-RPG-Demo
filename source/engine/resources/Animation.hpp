#ifndef PROTOTYPE_ACTION_RPG_ANIMATION_HPP
#define PROTOTYPE_ACTION_RPG_ANIMATION_HPP


#include <string>
#include <vector>
#include <functional>

#include "glm/glm.hpp"


namespace engine {

    class Animation {
    public:
        struct Sampler {
            enum InterpolationType { LINEAR, STEP, CUBICSPLINE };

            InterpolationType interpolation{};
            std::vector<float> inputs;
            std::vector<glm::vec4> outputs;
        };

        struct Channel {
            enum PathType { TRANSLATION, ROTATION, SCALE };

            PathType path{};
            int32_t nodeID{-1};
            uint32_t samplerIndex;
        };

    public:
        Animation();

    public:
        std::string m_name;
        std::vector<Sampler> m_samplers;
        std::vector<Channel> m_channels;
        float m_start{std::numeric_limits<float>::max()};
        float m_end{std::numeric_limits<float>::min()};
        float m_currentTime{1.0f};
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_ANIMATION_HPP
