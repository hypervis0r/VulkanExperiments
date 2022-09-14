#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <span>
#include <cstring>
#include <optional>
#include <unordered_set>
#include <string>
#include <limits>
#include <algorithm>
#include <memory>

#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "files.h"

#include "renderer/swapchain.h"
#include "renderer/queue.h"
#include "renderer/vertex.h"
#include "renderer/vulkanmem.h"
#include "renderer/uniform.h"

namespace Engine
{
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Renderer
	{
	private:
		GLFWwindow* Window;

		/*
			Vulkan-specific members
		*/
		const std::array<const char*, 1> ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::array<const char*, 1> DeviceExtentions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const int MAX_FRAMES_IN_FLIGHT = 2;

		vk::Instance VulkanInstance;
		bool ValidationLayersEnabled;
		vk::PhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		vk::Device LogicalDevice = VK_NULL_HANDLE;
		vk::SurfaceKHR Surface;
		VulkanQueues Queues;

		// smart ptr coz i want this initialized later
		std::shared_ptr<VulkanMemManager> MemManager;

		// Pipeline shit
		vk::Pipeline GraphicsPipeline;
		vk::PipelineLayout PipelineLayout;
		vk::RenderPass RenderPass;

		// Swap chain shit
		SwapChain Swapchain;

		// Command shit
		std::vector<vk::CommandBuffer> CommandBuffers;
		vk::CommandPool CommandPool;

		// Synch shit
		std::vector<vk::Semaphore> ImageAvailableSemaphores;
		std::vector<vk::Semaphore> RenderFinishedSemaphores;
		std::vector<vk::Fence> InFlightFences;
		uint32_t CurrentFrame = 0;

		// TEMP
		std::unique_ptr<VertexInputBuffer<Vertex>> vertexBuffer;
		std::unique_ptr<VertexInputBuffer<Index>> indexBuffer;

		// Uniform shit
		std::unique_ptr<VulkanDescriptorPool> DescriptorPool;
		std::vector<Uniform<UniformBufferObject>> Uniforms;

		void InitializeWindow();
		void MainRenderLoop();
		void Cleanup();
		void DrawFrame();

		/*
			Vulkan-specific functions
		*/
		template<std::size_t N>
		bool CheckVulkanLayerSupport(std::span<const char* const, N> layers);
		template<std::size_t N>
		bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, std::span<const char* const, N> extensions);
		void InitializeVulkan();
		void CreateVulkanInstance();
		bool IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateRenderSurface();
		void CreateGraphicsPipeline();
		void CreateRenderPass();

		// Shader shit
		template<std::size_t N>
		void CreateShaderModule(const std::span<char, N> code, vk::ShaderModule& module);

		// Command shit
		void CreateCommandBuffer();
		void CreateCommandPool();
		void RecordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t imageIndex);

		// Synch shit
		void CreateSyncObjects();

		void UpdateUniformWithNewData(Uniform<UniformBufferObject> Uniform);

	public:
		uint16_t WindowWidth;
		uint16_t WindowHeight;

		constexpr Renderer(bool EnableValidationLayers = false, uint16_t Width = 800, uint16_t Height = 600) :
			WindowWidth(Width),
			WindowHeight(Height),
			Window(nullptr),
			ValidationLayersEnabled(EnableValidationLayers) 
		{}

		void Run()
		{
			InitializeWindow();
			InitializeVulkan();
			MainRenderLoop();
			Cleanup();
		}
	};
}