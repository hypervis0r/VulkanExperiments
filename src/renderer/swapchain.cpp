#include "swapchain.h"

namespace Engine
{
	void SwapChain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, vk::SurfaceFormatKHR& format)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
				availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				format = availableFormat;
				return;
			}
		}

		format = availableFormats[0];
		return;
	}

	void SwapChain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, vk::PresentModeKHR& presentMode)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			{
				presentMode = availablePresentMode;
				return;
			}
		}

		// Fall back to FIFO
		presentMode = vk::PresentModeKHR::eFifo;
	}

	void SwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D& extent)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			extent = capabilities.currentExtent;
			return;
		}

		int width, height;
		glfwGetFramebufferSize(this->Window, &width, &height);

		vk::Extent2D actualExtent(
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		);

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);

		extent = actualExtent;
	}

	void SwapChain::CreateSwapChain(vk::SurfaceKHR& surface)
	{
		SwapChainSupportDetails details;
		VulkanDeviceContext::QuerySwapChainSupport(this->DeviceContext->PhysicalDevice, surface, details);

		vk::SurfaceFormatKHR surfaceFormat;
		vk::PresentModeKHR presentMode;
		vk::Extent2D extent;

		ChooseSwapSurfaceFormat(details.Formats, surfaceFormat);
		ChooseSwapPresentMode(details.PresentModes, presentMode);
		ChooseSwapExtent(details.Capabilities, extent);

		// Request one more than min so that we can acquire another image
		// before driver is finished.
		uint32_t imageCount = details.Capabilities.minImageCount + 1;

		// Ensure we dont go over the max image count
		if (details.Capabilities.maxImageCount > 0 &&
			imageCount > details.Capabilities.maxImageCount)
		{
			imageCount = details.Capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo(
			vk::SwapchainCreateFlagsKHR::Flags(),
			surface,
			imageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment);

		QueueFamilyIndices indices;
		indices.FindQueueFamilies(this->DeviceContext->PhysicalDevice, surface);

		const std::array<uint32_t, 2> queueFamilyIndices = { indices.GraphicsFamily.value(), indices.PresentationFamily.value() };

		if (indices.GraphicsFamily != indices.PresentationFamily)
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = details.Capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		// TODO: Come back to me later
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		this->Swapchain = this->DeviceContext->LogicalDevice.createSwapchainKHR(createInfo);
		this->SwapChainImages = this->DeviceContext->LogicalDevice.getSwapchainImagesKHR(this->Swapchain);
		this->SwapChainImageFormat = surfaceFormat.format;
		this->SwapChainExtent = extent;
	}

	void SwapChain::CreateImageViews()
	{
		vk::ImageViewCreateInfo createInfo{};

		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = this->SwapChainImageFormat;

		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;

		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		this->SwapChainImageViews.reserve(this->SwapChainImages.size());

		for (const auto& image : this->SwapChainImages)
		{
			createInfo.image = image;

			this->SwapChainImageViews.push_back(this->DeviceContext->LogicalDevice.createImageView(createInfo));
		}
	}

	void SwapChain::CreateFramebuffers(vk::RenderPass& renderPass)
	{
		this->SwapChainFramebuffers.reserve(this->SwapChainImageViews.size());

		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = this->SwapChainExtent.width;
		framebufferInfo.height = this->SwapChainExtent.height;
		framebufferInfo.layers = 1;

		for (const auto& imageView : this->SwapChainImageViews)
		{
			std::array<vk::ImageView, 1> attachments = {
				imageView
			};

			framebufferInfo.pAttachments = attachments.data();

			this->SwapChainFramebuffers.push_back(this->DeviceContext->LogicalDevice.createFramebuffer(framebufferInfo));
		}
	}

	void SwapChain::RecreateSwapChain(vk::SurfaceKHR& surface, vk::RenderPass& renderPass)
	{
		// If window is minimized, block application until its visible again
		int width = 0, height = 0;
		glfwGetFramebufferSize(this->Window, &width, &height);
		while (width == 0 || height == 0) 
		{
			glfwGetFramebufferSize(this->Window, &width, &height);
			glfwWaitEvents();
		}

		this->DeviceContext->LogicalDevice.waitIdle();

		Destroy();

		CreateSwapChain(surface);
		CreateImageViews();
		CreateFramebuffers(renderPass);
	}

	void SwapChain::Destroy()
	{
		// Framebuffers
		for (auto& framebuffer : this->SwapChainFramebuffers)
		{
			this->DeviceContext->LogicalDevice.destroyFramebuffer(framebuffer);
		}
		this->SwapChainFramebuffers.clear();

		// Image views
		for (auto& imageView : this->SwapChainImageViews)
		{
			this->DeviceContext->LogicalDevice.destroyImageView(imageView);
		}
		this->SwapChainImageViews.clear();

		// Swap chain
		this->DeviceContext->LogicalDevice.destroySwapchainKHR(this->Swapchain);
	}
}