#pragma once

#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "renderer/vulkandevicecontext.h"
#include "renderer/queue.h"

namespace Engine
{
	class SwapChain
	{
	private:
		std::shared_ptr<VulkanDeviceContext> DeviceContext;
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

		void CreateSwapChain(vk::SurfaceKHR& surface);
		void CreateImageViews();
		void CreateFramebuffers(vk::RenderPass& renderPass);
		void Destroy();
		void RecreateSwapChain(vk::SurfaceKHR& surface, vk::RenderPass& renderPass);

		SwapChain(std::shared_ptr<VulkanDeviceContext> devCtx, GLFWwindow* window)
			: DeviceContext(devCtx), Window(window) {}

		constexpr SwapChain()
			: DeviceContext(nullptr), Window(nullptr) {};
	};
}