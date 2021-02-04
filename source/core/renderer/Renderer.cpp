#include "Renderer.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Initializers.hpp"
#include "Tools.hpp"
#include "../model/Vertex.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"


namespace core {

    Renderer::Renderer(std::unique_ptr<Window>& window) : m_window(window) {
        init();
        initUI();

        spdlog::info("[Renderer] Initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::init() {
        VkApplicationInfo appInfo = vk::initializers::applicationInfo();
        appInfo.pApplicationName = "Prototype Action RPG";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = "Custom Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        std::vector<const char*> enabledExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_instance.init(appInfo);
        m_instance.createSurface(m_window->m_window, m_surface);
        m_instance.pickPhysicalDevice(m_physicalDevice, m_surface, enabledExtensions);

        m_device = new vk::Device(m_physicalDevice);

        VkPhysicalDeviceFeatures deviceFeatures{};
        vk::tools::validation(m_device->createLogicalDevice(deviceFeatures, enabledExtensions, vk::validationLayers),
                              "Failed to create logical device");

        m_logicalDevice = m_device->m_logicalDevice;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_device->m_queueFamilyIndices.graphics, m_surface, &presentSupport);

        if (presentSupport) {
            m_device->m_queueFamilyIndices.present = m_device->m_queueFamilyIndices.graphics;
        }

        vkGetDeviceQueue(m_logicalDevice, m_device->m_queueFamilyIndices.graphics, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, m_device->m_queueFamilyIndices.present, 0, &m_presentQueue);

        m_windowSize = m_window->getSize();
        m_swapChain.connect(m_physicalDevice, m_logicalDevice, m_surface);
        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.present);

        m_depthFormat= vk::tools::findSupportedFormat(m_physicalDevice,
                                                      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createCommandPool();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createTextureSampler();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSyncObjects();

        camera.getEye() = {0.5f, 0.5f, 0.5f};
        camera.getCenter() = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.getUp() = glm::vec3(0.0f, 1.0f, 0.0f);

        m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_size = glm::vec3(1.0f, 1.0f, 1.0f);
        m_angle = 0.0f;

        createMeshModel(MODELS_DIR + "viking-room.obj");
    }

    void Renderer::initUI() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF(std::string(FONTS_DIR + "Roboto-Medium.ttf").c_str(), 16.0f);

        createUIDescriptorPool();
        createUIRenderPass();
        createUICommandPool();
        createUIFramebuffers();

        // Provide bind points from Vulkan API
        ImGui_ImplGlfw_InitForVulkan(m_window->m_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = *m_instance;
        init_info.PhysicalDevice = m_physicalDevice;
        init_info.Device = m_logicalDevice;
        init_info.QueueFamily = m_device->m_queueFamilyIndices.graphics;
        init_info.Queue = m_graphicsQueue;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = m_uiDescriptorPool;
        init_info.MinImageCount = m_swapChain.getImageCount();
        init_info.ImageCount = m_swapChain.getImageCount();
        ImGui_ImplVulkan_Init(&init_info, m_uiRenderPass);

        // Upload the fonts for DearImgui
        VkCommandBuffer commandBuffer = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_uiCommandPool, true);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue, m_uiCommandPool, true);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void Renderer::cleanup() {
        vkDeviceWaitIdle(m_logicalDevice);

        vikingRoom.clean();

        for (auto& texture : textures) {
            texture.cleanup(m_logicalDevice);
        }

        // Cleanup DearImGui
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        cleanSwapChain();

        vkDestroySampler(m_logicalDevice, m_textureSampler, nullptr);

        vkDestroyDescriptorPool(m_logicalDevice, m_uiDescriptorPool, nullptr);
        vkDestroyCommandPool(m_logicalDevice, m_uiCommandPool, nullptr);
        vkDestroyRenderPass(m_logicalDevice, m_uiRenderPass, nullptr);

        vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_logicalDevice, samplerSetLayout, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice, m_fences[i], nullptr);
        }

        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

        m_device->destroy();
        delete m_device;

        m_instance.destroySurface(m_surface);
        m_instance.destroy();

        spdlog::info("[Renderer] Cleaned");
    }

    void Renderer::draw() {
        drawUI();
        drawFrame();
    }

    void Renderer::drawFrame() {
        vkWaitForFences(m_logicalDevice, 1, &m_fences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        uint32_t indexImage;
        VkResult result = m_swapChain.acquireNextImage(m_imageAvailableSemaphores[m_currentFrame], &indexImage);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw_ex("Failed to acquire swap chain image");
        }

        if (m_imageFences[indexImage] != VK_NULL_HANDLE) {
            vkWaitForFences(m_logicalDevice, 1, &m_imageFences[indexImage], VK_TRUE, std::numeric_limits<uint64_t>::max());
        }

        m_imageFences[indexImage] = m_fences[m_currentFrame];

        recordCommands(indexImage);
        recordUICommands(indexImage);
        updateUniformBuffer(indexImage);

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        std::array<VkCommandBuffer, 2> cmdBuffers = { m_commandBuffers[indexImage], m_uiCommandBuffers[indexImage] };

        VkSubmitInfo submitInfo = vk::initializers::submitInfo();
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
        submitInfo.pCommandBuffers = cmdBuffers.data();

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_logicalDevice, 1, &m_fences[m_currentFrame]);

        vk::tools::validation(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fences[m_currentFrame]),
                              "Failed to submit draw command buffer");

        VkSwapchainKHR swapChains[] = { m_swapChain.getSwapChain() };

        VkPresentInfoKHR presentInfo = vk::initializers::presentInfo();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &indexImage;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->m_resize) {
            m_window->m_resize = false;
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw_ex("Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::drawUI() {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

#ifdef CORE_DEBUG
        //        ImGui::ShowDemoWindow();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(330, 80), ImGuiCond_Always);
        ImGui::Begin("Application Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::Text("GPU: %s", m_device->m_properties.deviceName);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
#endif

        ImGui::SetNextWindowSize(ImVec2(220, -1), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(5, 90), ImGuiCond_Always);
        ImGui::Begin("Camera Control", nullptr, ImGuiWindowFlags_NoCollapse);
        {
            if (ImGui::CollapsingHeader("Eye")) {
                ImGui::InputFloat("Eye X", &camera.getEye().x);
                ImGui::InputFloat("Eye Y", &camera.getEye().y);
                ImGui::InputFloat("Eye Z", &camera.getEye().z);
            }

            if (ImGui::CollapsingHeader("Center")) {
                ImGui::InputFloat("Center X", &camera.getCenter().x);
                ImGui::InputFloat("Center Y", &camera.getCenter().y);
                ImGui::InputFloat("Center Z", &camera.getCenter().z);
            }

            if (ImGui::CollapsingHeader("Up")) {
                ImGui::InputFloat("Up X", &camera.getUp().x);
                ImGui::InputFloat("Up Y", &camera.getUp().y);
                ImGui::InputFloat("Up Z", &camera.getUp().z);
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(220, -1));
        ImGui::SetNextWindowPos(ImVec2(5, -1), ImGuiCond_Always);
        ImGui::Begin("Object", nullptr, ImGuiWindowFlags_NoCollapse);
        {
            if (ImGui::CollapsingHeader("Transform")) {
                if (ImGui::CollapsingHeader("Position")) {
                    ImGui::InputFloat("Position X", &m_position.x);
                    ImGui::InputFloat("Position Y", &m_position.y);
                    ImGui::InputFloat("Position Z", &m_position.z);
                }

                if (ImGui::CollapsingHeader("Size")) {
                    ImGui::InputFloat("Size X", &m_size.x);
                    ImGui::InputFloat("Size Y", &m_size.y);
                    ImGui::InputFloat("Size Z", &m_size.z);
                }

                if (ImGui::CollapsingHeader("Angle")) {
                    ImGui::InputFloat("Angle degree", &m_angle);
                }
            }
        }
        ImGui::End();

        ImGui::Render();
    }

    void Renderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChain.getFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = m_depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = vk::initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        renderPassInfo.pAttachments = attachments.data();;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::tools::validation(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass),
                              "Failed to create render pass");
    }

    void Renderer::createUIRenderPass() {
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.format = m_swapChain.getFormat();
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkAttachmentReference attachmentReference = {};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentReference;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = vk::initializers::renderPassCreateInfo();
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        vk::tools::validation(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_uiRenderPass),
                              "Unable to create UI render pass");
    }

    void Renderer::createGraphicsPipeline() {
        auto vertexShaderCode = core::tools::readFile("shaders/shader.vert.spv");
        auto fragmentShaderCode = core::tools::readFile("shaders/shader.frag.spv");

        VkShaderModule vertexShaderModule = vk::tools::loadShader(vertexShaderCode, m_logicalDevice);
        VkShaderModule fragmentShaderModule = vk::tools::loadShader(fragmentShaderCode, m_logicalDevice);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = vk::initializers::pipelineShaderStageCreateInfo();
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
                vertShaderStageInfo,
                fragShaderStageInfo
        };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk::initializers::pipelineVertexInputStateCreateInfo();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk::initializers::pipelineInputAssemblyStateCreateInfo();
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain.getExtent().width);
        viewport.height = static_cast<float>(m_swapChain.getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.getExtent();

        VkPipelineViewportStateCreateInfo viewportState = vk::initializers::pipelineViewportStateCreateInfo();
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = vk::initializers::pipelineRasterizationStateCreateInfo();
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampling = vk::initializers::pipelineMultisampleStateCreateInfo();
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlending = vk::initializers::pipelineColorBlendStateCreateInfo();
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::array<VkDescriptorSetLayout, 2> layouts = { m_descriptorSetLayout, samplerSetLayout };
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk::initializers::pipelineLayoutCreateInfo();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        vk::tools::validation(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
                              "Failed to create pipeline layout");

        VkPipelineDepthStencilStateCreateInfo depthStencil = vk::initializers::pipelineDepthStencilStateCreateInfo();
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkGraphicsPipelineCreateInfo pipelineInfo = vk::initializers::graphicsPipelineCreateInfo();
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        vk::tools::validation(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline),
                              "Failed to create graphics pipeline");

        vkDestroyShaderModule(m_logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_logicalDevice, fragmentShaderModule, nullptr);
    }

    void Renderer::createFramebuffers() {
        m_framebuffers.resize(m_swapChain.getImageCount());

        for (size_t i = 0; i < m_swapChain.getImageCount(); ++i) {
            std::array<VkImageView, 2> attachments = {
                    m_swapChain.getImageView(i),
                    m_depthBuffer.getView()
            };

            VkFramebufferCreateInfo framebufferInfo = vk::initializers::framebufferCreateInfo();
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapChain.getExtent().width;
            framebufferInfo.height = m_swapChain.getExtent().height;
            framebufferInfo.layers = 1;

            vk::tools::validation(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]),
                                  "Failed to create framebuffer");
        }
    }

    void Renderer::createUIFramebuffers() {
        m_uiFramebuffers.resize(m_swapChain.getImageCount());

        VkImageView attachment[1];
        VkFramebufferCreateInfo info = vk::initializers::framebufferCreateInfo();
        info.renderPass = m_uiRenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = m_swapChain.getExtent().width;
        info.height = m_swapChain.getExtent().height;
        info.layers = 1;

        for (uint32_t i = 0; i < m_swapChain.getImageCount(); ++i) {
            attachment[0] = m_swapChain.getImageView(i);

            vk::tools::validation(vkCreateFramebuffer(m_logicalDevice, &info, nullptr, &m_uiFramebuffers[i]),
                                  "Unable to create UI framebuffers");
        }
    }

    void Renderer::createCommandPool() {
        m_commandPool = m_device->createCommandPool(m_device->m_queueFamilyIndices.graphics);

        m_commandBuffers.resize(m_swapChain.getImageCount());

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }
    }

    void Renderer::createUICommandPool() {
        m_uiCommandPool = m_device->createCommandPool(m_device->m_queueFamilyIndices.graphics);

        m_uiCommandBuffers.resize(m_swapChain.getImageCount());

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_uiCommandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_uiCommandPool);
        }
    }

    void Renderer::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_fences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imageFences.resize(m_swapChain.getImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = vk::initializers::semaphoreCreateInfo();

        VkFenceCreateInfo fenceInfo = vk::initializers::fenceCreateInfo();
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vk::tools::validation(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]),
                                  "Failed to create semaphores");

            vk::tools::validation(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]),
                                  "Failed to create semaphores");

            vk::tools::validation(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_fences[i]),
                                  "Failed to create a fence");
        }
    }

    void Renderer::recordCommands(uint32_t bufferIdx) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        VkRenderPassBeginInfo renderPassInfo = vk::initializers::renderPassBeginInfo();

        VkCommandBufferBeginInfo beginInfo = vk::initializers::commandBufferBeginInfo();
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk::tools::validation(vkBeginCommandBuffer(m_commandBuffers[bufferIdx], &beginInfo),
                              "Failed to begin recording command buffer");
        {
            renderPassInfo.renderPass = m_renderPass;
            renderPassInfo.framebuffer = m_framebuffers[bufferIdx];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_swapChain.getExtent();
            renderPassInfo.renderArea.extent.width = m_swapChain.getExtent().width;
            renderPassInfo.renderArea.extent.height = m_swapChain.getExtent().height;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[bufferIdx], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(m_commandBuffers[bufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

                for (int i = 0; i < vikingRoom.getMeshCount(); ++i) {
                    VkBuffer vertexBuffer[] = { vikingRoom.getMesh(i)->getVertexBuffer() };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(m_commandBuffers[bufferIdx], 0, 1, vertexBuffer, offsets);
                    vkCmdBindIndexBuffer(m_commandBuffers[bufferIdx], vikingRoom.getMesh(i)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

                    std::array<VkDescriptorSet, 2> descriptorSetGroup = {
                            m_descriptorSets[bufferIdx],
                            textures[vikingRoom.getMesh(i)->getTextureId()].getDescriptorSet()
                    };

                    vkCmdBindDescriptorSets(m_commandBuffers[bufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
                                            0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

                    vkCmdBindDescriptorSets(m_commandBuffers[bufferIdx], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            m_pipelineLayout, 0, 1, &m_descriptorSets[bufferIdx], 0, nullptr);

                    vkCmdDrawIndexed(m_commandBuffers[bufferIdx], vikingRoom.getMesh(i)->getIndexCount(),
                                     1, 0, 0, 0);
                }
            }
            vkCmdEndRenderPass(m_commandBuffers[bufferIdx]);
        }
        vk::tools::validation(vkEndCommandBuffer(m_commandBuffers[bufferIdx]),
                              "Failed to record command buffer");
    }

    void Renderer::recordUICommands(uint32_t bufferIdx) {
        VkCommandBufferBeginInfo cmdBufferBegin = vk::initializers::commandBufferBeginInfo();
        cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk::tools::validation(vkBeginCommandBuffer(m_uiCommandBuffers[bufferIdx], &cmdBufferBegin),
                              "Unable to start recording UI command buffer!");

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        VkRenderPassBeginInfo renderPassBeginInfo = vk::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = m_uiRenderPass;
        renderPassBeginInfo.framebuffer = m_uiFramebuffers[bufferIdx];
        renderPassBeginInfo.renderArea.extent.width = m_swapChain.getExtent().width;
        renderPassBeginInfo.renderArea.extent.height = m_swapChain.getExtent().height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_uiCommandBuffers[bufferIdx], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // Grab and record the draw data for Dear Imgui
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_uiCommandBuffers[bufferIdx]);
        }
        // End and submit render pass
        vkCmdEndRenderPass(m_uiCommandBuffers[bufferIdx]);

        vk::tools::validation(vkEndCommandBuffer(m_uiCommandBuffers[bufferIdx]),
                              "Failed to record command buffers");
    }

    void Renderer::recreateSwapchain() {
        // TODO: Optimize window resize
        while (m_windowSize.width == 0 || m_windowSize.height == 0)  {
            m_windowSize = m_window->getSize();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_logicalDevice);

        cleanSwapChain();

        m_swapChain.create(m_windowSize.width, m_windowSize.height, m_device->m_queueFamilyIndices.graphics, m_device->m_queueFamilyIndices.present);
        createRenderPass();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_commandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool);
        }

        // We also need to take care of the UI
        ImGui_ImplVulkan_SetMinImageCount(m_swapChain.getImageCount());
        m_uiCommandBuffers.resize(m_swapChain.getImageCount());

        for (int i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_uiCommandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_uiCommandPool);
        }

        createUIFramebuffers();
    }

    void Renderer::cleanSwapChain() {
        m_depthBuffer.cleanup(m_logicalDevice);

        for (auto & framebuffer : m_uiFramebuffers) {
            vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
        }

        for (auto & framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
        }

        vkFreeCommandBuffers(m_logicalDevice, m_uiCommandPool, static_cast<uint64_t>(m_uiCommandBuffers.size()),
                             m_uiCommandBuffers.data());

        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, static_cast<uint64_t>(m_commandBuffers.size()),
                             m_commandBuffers.data());

        vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

        m_swapChain.cleanup();

        for (size_t i = 0; i < m_swapChain.getImageCount(); i++) {
            vkDestroyBuffer(m_logicalDevice, m_uniformBuffers[i].m_buffer, nullptr);
            vkFreeMemory(m_logicalDevice, m_uniformBuffers[i].m_memory, nullptr);
        }

        vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
        vkDestroyDescriptorPool(m_logicalDevice, samplerDescriptorPool, nullptr);
    }

    void Renderer:: createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo uboLayoutInfo = vk::initializers::descriptorSetLayoutCreateInfo();
        uboLayoutInfo.bindingCount = 1;
        uboLayoutInfo.pBindings = &uboLayoutBinding;

        vk::tools::validation(vkCreateDescriptorSetLayout(m_logicalDevice, &uboLayoutInfo, nullptr, &m_descriptorSetLayout),
                              "Failed to create descriptor set layout");

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo samplerLayoutInfo = vk::initializers::descriptorSetLayoutCreateInfo();
        samplerLayoutInfo.bindingCount = 1;
        samplerLayoutInfo.pBindings = &samplerLayoutBinding;

        vk::tools::validation(vkCreateDescriptorSetLayout(m_logicalDevice, &samplerLayoutInfo, nullptr, &samplerSetLayout),
                              "Failed to create descriptor set layout");
    }

    void Renderer::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(m_swapChain.getImageCount());

        for (size_t i = 0; i < m_swapChain.getImageCount(); ++i) {
            m_device->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &m_uniformBuffers[i],
                                   bufferSize);
        }
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        glm::mat4 model(1.0f);
        model = glm::translate(model, m_position);
        model = glm::scale(model, m_size);
        model = glm::rotate(model, glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f));

        ubo.model = model;

        ubo.view = camera.getView();

        ubo.proj = glm::perspective(glm::radians(45.0f), m_window->aspect(), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        m_uniformBuffers[currentImage].map(sizeof(ubo), 0);
        m_uniformBuffers[currentImage].copyTo(&ubo, sizeof(ubo));
        m_uniformBuffers[currentImage].unmap();
    }

    void Renderer::createDescriptorPool() {
        VkDescriptorPoolSize uniformPoolSize{};
        uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformPoolSize.descriptorCount = m_swapChain.getImageCount();

        VkDescriptorPoolCreateInfo uniformPoolInfo = vk::initializers::descriptorPoolCreateInfo();
        uniformPoolInfo.poolSizeCount = 1;
        uniformPoolInfo.pPoolSizes = &uniformPoolSize;
        uniformPoolInfo.maxSets = m_swapChain.getImageCount();
        uniformPoolInfo.flags = 0;

        vk::tools::validation(vkCreateDescriptorPool(m_logicalDevice, &uniformPoolInfo, nullptr, &m_descriptorPool),
                              "Failed to create descriptor pool");

        VkDescriptorPoolSize samplerPoolSizer{};
        samplerPoolSizer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerPoolSizer.descriptorCount = MAX_OBJECTS;

        VkDescriptorPoolCreateInfo samplerPoolCreateInfo{};
        samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
        samplerPoolCreateInfo.poolSizeCount = 1;
        samplerPoolCreateInfo.pPoolSizes = &samplerPoolSizer;

        vk::tools::validation(vkCreateDescriptorPool(m_logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool),
                              "Failed to create a Descriptor Pool");
    }

    void Renderer::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(m_swapChain.getImageCount(), m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = vk::initializers::descriptorSetAllocateInfo();
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = m_swapChain.getImageCount();
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapChain.getImageCount());
        vk::tools::validation(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()),
                              "Failed to allocate descriptor sets");

        for (size_t i = 0; i < m_swapChain.getImageCount(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i].m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
            descriptorWrites[0] = vk::initializers::writeDescriptorSet();
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(), 0, nullptr);
        }
    }

    void Renderer::createUIDescriptorPool() {
        VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * std::size(pool_sizes);
        pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        vk::tools::validation(vkCreateDescriptorPool(m_logicalDevice, &pool_info, nullptr, &m_uiDescriptorPool),
                              "Cannot allocate UI descriptor pool");
    }

    uint Renderer::createTexture(const std::string& fileName) {
        int width, height;
        VkDeviceSize imageSize;
        stbi_uc* pixels = core::tools::loadTextureFile(fileName, &width, &height, &imageSize);

        vk::Buffer stagingBuffer;

        m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &stagingBuffer, imageSize);

        stagingBuffer.map(imageSize);
        stagingBuffer.copyTo(pixels, imageSize);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        auto u32Width = static_cast<uint32_t>(width);
        auto u32Height = static_cast<uint32_t>(height);

        core::Texture texture(m_logicalDevice, u32Width, u32Height,
                              VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_logicalDevice, texture.getTextureImage().getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        texture.bind(m_logicalDevice, memType, memoryRequirements.size);

        transitionImageLayout(texture.getTextureImage().getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer.m_buffer, texture.getTextureImage().getImage(), u32Width, u32Height);
        transitionImageLayout(texture.getTextureImage().getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        stagingBuffer.destroy();

        texture.createDescriptor(m_logicalDevice, samplerDescriptorPool, m_textureSampler, samplerSetLayout);

        textures.push_back(texture);

        return static_cast<uint>(textures.size()) - 1;
    }

    void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool, true);

        VkImageMemoryBarrier barrier = vk::initializers::imageMemoryBarrier();
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // Check for stencil support
            if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
           core::throw_ex("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue, m_commandPool, true);
    }

    void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool, true);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );

        m_device->flushCommandBuffer(commandBuffer, m_graphicsQueue, m_commandPool);
    }

    void Renderer::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo = vk::initializers::samplerCreateInfo();
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        vk::tools::validation(vkCreateSampler(m_logicalDevice, &samplerInfo, nullptr, &m_textureSampler),
                              "Failed to create texture sampler");
    }

    void Renderer::createDepthResources() {


        VkImageCreateInfo imageInfo = vk::initializers::imageCreateInfo();
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_swapChain.getExtent().width;
        imageInfo.extent.height = m_swapChain.getExtent().height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        m_depthBuffer = vk::Image(m_logicalDevice, imageInfo);

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_logicalDevice, m_depthBuffer.getImage(), &memoryRequirements);

        auto memType = m_device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_depthBuffer.bind(m_logicalDevice, memType, memoryRequirements.size, VK_IMAGE_ASPECT_DEPTH_BIT);

        transitionImageLayout(m_depthBuffer.getImage(), m_depthBuffer.getFormat(), VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    void Renderer::createMeshModel(const std::string &modelFile) {
        // Import model "scene"
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if (!scene) {
            throw std::runtime_error("Failed to load model! (" + modelFile + ")");
        }

        // Get vector of all materials with 1:1 ID placement
        std::vector<std::string> textureNames = core::Model::loadMaterials(scene);

        // Conversion from the materials list IDs to our Descriptor Array IDs
        std::vector<uint> matToTex(textureNames.size());

        // Loop over textureNames and create textures for them
        for (size_t i = 0; i < textureNames.size(); i++) {
            // If material had no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
            if (textureNames[i].empty()) {
                matToTex[i] = 0;
            } else {
                // Otherwise, create texture and set value to index of new texture
                matToTex[i] = createTexture(textureNames[i]);
            }
        }

        // Load in all our meshes
        std::vector<core::Mesh> modelMeshes = core::Model::LoadNode(m_device, m_graphicsQueue, scene->mRootNode, scene, matToTex);

        // Create mesh model and add to list
        vikingRoom = core::Model(modelMeshes);
    }

} // End namespace core