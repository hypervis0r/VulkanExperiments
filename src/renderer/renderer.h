#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <span>
#include <cstring>
#include <optional>
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

#include "renderer/vulkandevicecontext.h"
#include "renderer/swapchain.h"
#include "renderer/queue.h"
#include "renderer/vertex.h"
#include "renderer/vulkanmem.h"
#include "renderer/uniform.h"
#include "renderer/image.h"

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

		const int MAX_FRAMES_IN_FLIGHT = 2;

		bool ValidationLayersEnabled;

		std::shared_ptr<VulkanDeviceContext> DeviceContext;


		// Pipeline shit
		vk::Pipeline GraphicsPipeline;
		vk::PipelineLayout PipelineLayout;
		vk::RenderPass RenderPass;

		// Swap chain shit
		SwapChain Swapchain;

		// Command shit
		std::vector<vk::CommandBuffer> CommandBuffers;
		

		// Synch shit
		std::vector<vk::Semaphore> ImageAvailableSemaphores;
		std::vector<vk::Semaphore> RenderFinishedSemaphores;
		std::vector<vk::Fence> InFlightFences;
		uint32_t CurrentFrame = 0;

		// TEMP
		std::unique_ptr<VertexInputBuffer<Vertex>> vertexBuffer;
		std::unique_ptr<VertexInputBuffer<Index>> indexBuffer;
		std::unique_ptr<Image> Texture;

		// Uniform shit
		std::unique_ptr<VulkanDescriptorPool> DescriptorPool;
		std::vector<Uniform<UniformBufferObject>> Uniforms;

		void InitializeWindow();
		void InitializeVulkan();
		void MainRenderLoop();
		void Cleanup();
		void DrawFrame();

		/*
			Vulkan-specific functions
		*/
		
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