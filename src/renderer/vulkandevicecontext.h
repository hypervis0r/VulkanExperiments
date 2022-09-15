#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <unordered_set>

#include "renderer/vulkanmem.h"
#include "renderer/queue.h"

namespace Engine
{
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	class VulkanDeviceContext
	{
	private:
		/*
			Vulkan-specific members
		*/
		const std::array<const char*, 1> ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::array<const char*, 1> DeviceExtentions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		template<std::size_t N>
		bool CheckVulkanLayerSupport(std::span<const char* const, N> layers);
		template<std::size_t N>
		bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, std::span<const char* const, N> extensions);
		void InitializeVulkan();
		void CreateVulkanInstance();
		bool IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();
		void CreateRenderSurface(GLFWwindow* Window);

	public:
		vk::Device LogicalDevice;
		vk::PhysicalDevice PhysicalDevice;
		std::unique_ptr<VulkanMemManager> MemManager;
		vk::CommandPool CommandPool;

		VulkanQueues Queues;
		vk::SurfaceKHR Surface;

		vk::Instance VulkanInstance;
		bool ValidationLayersEnabled;

		static void QuerySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface, SwapChainSupportDetails& details);

		VulkanDeviceContext(GLFWwindow* Window, bool EnableValidationLayers) :
			ValidationLayersEnabled(EnableValidationLayers)
		{
			CreateVulkanInstance();
			CreateRenderSurface(Window);
			PickPhysicalDevice();
			CreateLogicalDevice();

			CreateCommandPool();

			this->MemManager = std::make_unique<VulkanMemManager>(this->LogicalDevice, this->PhysicalDevice, this->CommandPool, this->Queues);
		}
	};
}