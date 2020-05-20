#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "easylogging++.h"

#include "BSPTextureParser.h"
#include "Camera.h"
#include "FileSystem.h"
#include "Mesh.h"
#include "Platform.h"
#include "Vertex.h"
#include "VulkanImage.h"

#include "util.h"

using std::runtime_error;
using std::string;
using std::vector;

class VulkanApplication {
    public:
        VulkanApplication(const Platform&, Camera*, Mesh&, BSPTextureParser*);
        virtual ~VulkanApplication();

        uint32_t getEnabledExtensionCount();
        const char** getEnabledExtensions();

        uint32_t getEnabledLayerCount();
        const char** getEnabledLayers();

        VkSurfaceCapabilitiesKHR getSurfaceCapabilities();

        void present();
        void resize();
    
    protected:
        vector<string> _enabledExtensions;
        vector<string> _enabledLayers;
        bool _shouldResize;

        Camera* _camera;
        BSPTextureParser* _atlas;
        Mesh& _mesh;

        VulkanImage depth;
        vector<VulkanSampler> textureSamplers;
        vector<VulkanSampler> lightMapSamplers;

        VkInstance _instance;
        VkDebugReportCallbackEXT _debugCallback;
        VkPhysicalDevice _physicalDevice;
        uint32_t _gfxFamily;
        VkQueue _gfxQueue;
        VkDevice _device;
        VkPhysicalDeviceMemoryProperties _memories;
        uint32_t _version;
        VkSurfaceKHR _surface;
        VkPresentModeKHR _presentMode;
        VkSurfaceCapabilitiesKHR _surfaceCapabilities;
        VkSwapchainKHR _swapChain;
        VkExtent2D _swapChainExtent;
        VkFormat _swapImageFormat;
        VkColorSpaceKHR _swapImageColorSpace;
        vector<VkImage> _swapImages;
        vector<VkImageView> _swapImageViews;
        VkCommandPool _graphicsCommandPool;
        VkCommandPool _transientCommandPool;
        VkRenderPass _renderPass;
        vector<VkFramebuffer> _framebuffers;
        VkShaderModule _vertexShader;
        VkShaderModule _fragmentShader;
        VkPipelineLayout _layout;
        VkPipeline _pipeline;
        vector<VkCommandBuffer> _swapCommandBuffers;
        VkSemaphore _imageReady;
        VkSemaphore _presentReady;

        VkDescriptorSetLayout _descriptorSetLayout;
        VkDescriptorPool _descriptorPool;
        VkDescriptorSet _descriptorSet;

        VkBuffer _vertexBuffer;
        VkBuffer _mvpBuffer;
        VkBuffer _lightMapBuffer;
        VkBufferView lightMapView;

        VkDeviceMemory _lightMapMemory;
        VkDeviceMemory _vertexMemory;
        VkDeviceMemory _uniformMemory;

        void checkSuccess(VkResult result, const string& errorMessage);
        void checkVersion(uint32_t version);

        void getMemories();

        VkDeviceMemory allocateBuffer(VkBuffer);
        void allocateUniformBuffer();
        void allocateVertexBuffer();

        VkBuffer createBuffer(VkBufferUsageFlags, uint32_t);
        void createLightMapBuffer();
        void createMVPBuffer();
        void createVertexBuffer();

        void uploadUniformData();
        void uploadVertexData();
        void uploadTextures();
        void uploadTexture(
            uint32_t,
            uint32_t,
            std::vector<uint8_t>&,
            VulkanSampler&
        );

        void createDescriptorPool();
        void allocateDescriptorSet();
        void updateDescriptorSet();

        void createVulkanInstance();
        void createDebugCallback();
        void createPhysicalDevice();
        void createDeviceAndQueues();
        void createSwapChain();
        void createRenderPass();
        void createFramebuffers();
        VkShaderModule createShaderModule(const string&);
        VkShaderModule createShaderModule(const vector<char>& code);
        void createPipeline(VkShaderModule&, VkShaderModule&);
        void createCommandPools();
        void createSwapCommandBuffers();
        void createSemaphores();

        void checkSurfaceCapabilities();

        void destroyFramebuffers();
        void destroySwapchain(VkSwapchainKHR&);
        void destroySwapImageViews();

        void getSwapImagesAndImageViews();

        void initCamera();

        void recordCommandBuffers();
        void resizeSwapChain();
};
