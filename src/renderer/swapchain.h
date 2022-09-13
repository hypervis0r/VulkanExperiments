#pragma once

#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "renderer/queuefamily.h"

namespace Engine
{
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	class SwapChain
	{
	private:
		vk::PhysicalDevice PhysicalDevice;
		vk::Device LogicalDevice;
		GLFWwindow* Window;

		// Swap chain shit
		void ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, vk::SurfaceFormatKHR& format);
		void ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, vk::PresentModeKHR& presentMode);
		void ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D& extent);

	public:
		vk::SwapchainKHR Swapchain;
		std::vector<vk::Image> SwapChainImages;
		vk::Format SwapChainImageFormat = vk::Format::eUndefined;
		vk::Extent2D SwapChainExtent;
		std::vector<vk::ImageView> SwapChainImageViews;
		std::vector<vk::Framebuffer> SwapChainFramebuffers;
		
		static void QuerySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface, SwapChainSupportDetails& details);

		void CreateSwapChain(vk::SurfaceKHR& surface);
		void CreateImageViews();
		void CreateFramebuffers(vk::RenderPass& renderPass);
		void Destroy();
		void RecreateSwapChain(vk::SurfaceKHR& surface, vk::RenderPass& renderPass);

		constexpr SwapChain(vk::PhysicalDevice& physicalDevice, vk::Device& device, GLFWwindow* window)
			: LogicalDevice(device), PhysicalDevice(physicalDevice), Window(window) {}

		constexpr SwapChain()
			: LogicalDevice(VK_NULL_HANDLE), PhysicalDevice(VK_NULL_HANDLE), Window(nullptr) {};
	};
}