#include "AnimationInterface.hpp"

#include <utility>

#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "../Application.hpp"
#include "../resources/Model.hpp"


namespace engine {

    AnimationInterface::AnimationInterface() = default;

    AnimationInterface::AnimationInterface(std::shared_ptr<Model> model, std::vector<uint32_t> animationList)
            : model(std::move(model)), animationsList(std::move(animationList)) {

    }

    void AnimationInterface::update(float delaTime) {
        animation = Application::m_resourceManager->getAnimation(animationsList[currentAnimation - 1]);
        animation->m_currentTime += delaTime;

        if (animation->m_currentTime > animation->m_end) animation->m_currentTime -= animation->m_end;

        for (auto& channel : animation->m_channels) {
            Animation::Sampler& sampler = animation->m_samplers[channel.samplerIndex];

            if (sampler.interpolation != Animation::Sampler::InterpolationType::LINEAR) {
                spdlog::error( "This sample only supports linear interpolations\n");
                continue;
            }

            for (size_t i = 0; i < sampler.inputs.size() - 1; ++i) {
                if ((animation->m_currentTime >= sampler.inputs[i]) && (animation->m_currentTime <= sampler.inputs[i + 1])) {
                    float a = (animation->m_currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                    Model::Node& node = model->getNode(channel.nodeID);

                    switch (channel.path) {
                        case Animation::Channel::PathType::TRANSLATION: {
                            node.position = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], a);
                        }
                        case Animation::Channel::PathType::ROTATION: {
                            glm::quat q1;
                            q1.x = sampler.outputs[i].x;
                            q1.y = sampler.outputs[i].y;
                            q1.z = sampler.outputs[i].z;
                            q1.w = sampler.outputs[i].w;

                            glm::quat q2;
                            q2.x = sampler.outputs[i + 1].x;
                            q2.y = sampler.outputs[i + 1].y;
                            q2.z = sampler.outputs[i + 1].z;
                            q2.w = sampler.outputs[i + 1].w;

                            node.rotation = glm::normalize(glm::slerp(q1, q2, a));
                        }
                        case Animation::Channel::PathType::SCALE: {
                            node.scale = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], a);
                        }
                    }
                }
            }
        }

        for (auto& node : model->getNodes()) {
            if (node.mesh > 0) {
                glm::mat4 matrix = node.getMatrix(model);
                Mesh& mesh = Application::m_resourceManager->getMesh(node.mesh);

                if (node.skin > -1) {
                    mesh.m_uniformBlock.matrix = matrix;
                    glm::mat4 inverseTransform = glm::inverse(matrix);
                    Model::Skin &skin = model->getSkin(node.skin);
                    size_t numJoints = static_cast<uint32_t>(skin.joints.size());

                    for (size_t i = 0; i < numJoints; ++i) {
                        glm::mat4 jointMatrix = model->getNode(skin.joints[i]).getMatrix(model) * skin.inverseBindMatrices[i];
                        mesh.m_uniformBlock.jointMatrix[i] = inverseTransform * jointMatrix;
                    }

                    mesh.m_uniformBlock.jointCount = (float)numJoints;
                    mesh.m_uniformBuffer.copyTo(&mesh.m_uniformBlock, sizeof(mesh.m_uniformBlock));
                } else {
                    mesh.m_uniformBuffer.copyTo(&matrix, sizeof(glm::mat4));
                }
            }
        }
    }

    void AnimationInterface::setLuaBindings(sol::table &table) {
        table.new_enum("AnimationType",
                       "idle", Animation::Type::idle,
                       "attack", Animation::Type::attack,
                       "death", Animation::Type::death,
                       "walk", Animation::Type::walk);

        table.new_usertype<AnimationInterface>("AnimationInterface",
                                               sol::call_constructor, sol::constructors<AnimationInterface()>(),
                                               "currentType", &AnimationInterface::currentAnimation,
                                               "animationsList", &AnimationInterface::animationsList);
    }

} // namespace engine