#include "Application.hpp"

#include "spdlog/spdlog.h"

#include "renderer/CommandList.hpp"
#include "renderer/GraphicsPipeline.hpp"
#include "lua/MathBindings.hpp"


namespace engine {

    std::unique_ptr<engine::RenderEngine> Application::m_renderer;
    std::unique_ptr<engine::ResourceManager> Application::m_resourceManager;
    std::unique_ptr<engine::Scene> Application::m_scene;
    std::unique_ptr<ThreadPool> Application::m_threadPool;
    bool Application::m_editor;

    Application::Application(const std::string& appName, const glm::vec4& clearColor, bool editor)
            : m_clearColor(clearColor) {
        spdlog::info("[App] Start");
        m_editor = editor;

        m_window = std::make_shared<engine::Window>(appName, 1776, 1000);

        m_instance = std::make_shared<engine::Instance>(vk::ApplicationInfo{
            .pApplicationName = appName.c_str(),
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "Custom Engine",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0)
        });

        m_device = std::make_shared<engine::Device>(m_instance);
        m_renderer = std::make_unique<engine::RenderEngine>(m_window, m_instance->getInstance(), appName, m_device, m_instance->createSurface(m_window->getWindow()));
        m_resourceManager = std::make_unique<engine::ResourceManager>(m_device, m_renderer->getGraphicsQueue());

        vk::PushConstantRange constantRange{
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .offset = 0,
                .size = sizeof(MVP)
        };

        std::string vertShader = std::string(m_editor ? "editor" : "animation") + ".vert.spv";
        std::string fragShader = std::string(m_editor ? "editor" : "animation") + ".frag.spv";
        m_pipelineAnimation = m_renderer->addPipeline(Application::m_resourceManager->createShader(vertShader, fragShader, {constantRange}),
                                                      m_device->m_logicalDevice);
        m_renderer->init();

        m_scene = std::make_unique<engine::Scene>();
        m_commands = m_renderer->addCommandList();

        m_ui = engine::UIRender(m_renderer->getSwapChain(), m_device, m_window->getWindow(), m_instance->getInstance(), m_renderer->getGraphicsQueue(),
                               m_renderer->addCommandList());

        m_window->setLuaBindings(m_luaManager.getState());
        lua::setMathBindings(m_luaManager.getState());
        m_ui.setLuaBindings(m_luaManager.getState());
        m_scene->setLuaBindings(m_luaManager.getState());

        sol::table tools = m_luaManager.getState()["tools"].get_or_create<sol::table>();
        tools.set_function("hashString", &tools::hashString);
        tools.set_function("getDeltaTime", &Application::getDeltaTime, this);

        m_threadPool = std::make_unique<ThreadPool>();

        spdlog::info("[App] Start");
    }

    Application::~Application() = default;

    void Application::run() {
        init();

        if (!m_editor) updatePipeline();

        loop();
        shutdown();
    }

    void Application::shutdown() {
        vkDeviceWaitIdle(m_device->m_logicalDevice);

        cleanup();
        m_threadPool->stop();
        m_scene->cleanup();
        m_ui.cleanupResources();
        m_ui.cleanup();
        m_renderer->cleanup(m_instance);
        m_resourceManager->cleanup();
        m_device->destroy();
        m_instance->destroy();
        m_window->clean();

        spdlog::info("[App] Cleaned");
    }

    void Application::loop() {
        while (m_window->isOpen()) {
            glfwPollEvents();

            auto now = static_cast<float>(glfwGetTime());
            m_deltaTime = now - m_lastTime;
            m_lastTime = now;

            m_renderer->updateVP(m_scene->getCamera().getView(), m_scene->getCamera().getProjection(m_window->aspect()));
            m_scene->update(m_deltaTime);

            if (!m_editor) {
                m_threadPool->submit([animation = &m_resourceManager->getAnimation(3451133277237452101), deltaTime = m_deltaTime,
                                      model = m_resourceManager->getModel(2712)] {
                    animation->m_currentTime += deltaTime;

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
                            Mesh& mesh = m_resourceManager->getMesh(node.mesh);

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
                });
            }

            update();

            engine::UIRender::newFrame();
            drawUI();
            m_luaManager.executeFunction("drawUI");
            engine::UIRender::render();

            m_renderer->acquireNextImage();
            m_commands->begin();
            {
                m_commands->beginRenderPass(m_renderer->getRenderPass(), m_clearColor, m_renderer->getFrameBuffer(), m_renderer->getSwapChainExtent());
                {
                    m_pipelineAnimation->bind(m_commands->getBuffer());
                    m_commands->getBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineAnimation->getLayout(), 0, 1,
                                                               &m_renderer->getDescriptorSet(), 0, nullptr);
                    m_scene->render(m_commands->getBuffer(), m_pipelineAnimation);
                    renderCommands(m_commands->getBuffer());
                }
                m_commands->endRenderPass();
            }
            m_commands->end();
            m_ui.recordCommands(m_renderer->getImageIndex(), m_renderer->getSwapChainExtent());
            m_renderer->render();
        }
    }

    float Application::getDeltaTime() const {
        return m_deltaTime;
    }

    void Application::updatePipeline() {
        uint32_t maxPoolSize = m_resourceManager->getMeshesCount();
        vk::DescriptorPoolSize poolSizes = {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = maxPoolSize
        };

        m_resourceManager->createMeshDescriptors({poolSizes}, maxPoolSize + 1);
        m_resourceManager->createMeshDescriptorSets();

        std::vector<vk::DescriptorSetLayout> layouts = {
                m_renderer->getDescriptorSetLayout(),
                m_resourceManager->getTextureDescriptorSetLayout(),
                m_resourceManager->getMeshDescriptorSetLayout()
        };

        m_pipelineAnimation->cleanup();
        m_pipelineAnimation->create(layouts, m_renderer->getSwapChain(), m_renderer->getRenderPass(), m_device->getMaxUsableSampleCount());
    }

} // namespace core