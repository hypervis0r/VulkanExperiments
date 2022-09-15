#include "renderer/vulkandevicecontext.h"

namespace Engine
{
	void VulkanDeviceContext::QuerySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface, SwapChainSupportDetails& details)
	{
		vk::resultCheck(device.getSurfaceCapabilitiesKHR(surface, &details.Capabilities),
			"Failed to get device surface capabilities.");

		details.Formats = device.getSurfaceFormatsKHR(surface);
		details.PresentModes = device.getSurfacePresentModesKHR(surface);
	}

	template<std::size_t N>
	bool VulkanDeviceContext::CheckVulkanLayerSupport(const std::span<const char* const, N> layers)
	{
		bool layerFound = false;
		auto availableLayers = vk::enumerateInstanceLayerProperties();

		for (const char* layerName : layers)
		{
			layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (std::strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	void VulkanDeviceContext::CreateVulkanInstance()
	{
		if (this->ValidationLayersEnabled && !CheckVulkanLayerSupport(std::span{ this->ValidationLayers }))
			throw std::runtime_error("Validation layers requested, but are not available.");

		vk::ApplicationInfo appInfo(
			"Hello Triangle",
			VK_MAKE_VERSION(1, 0, 0),
			"No Engine",
			VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0);

		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		if (glfwExtensions == nullptr || glfwExtensionCount == 0)
			throw std::runtime_error("Failed to get required GLFW instance extensions");

		vk::InstanceCreateInfo createInfo(
			{},
			&appInfo,
			0, nullptr,
			glfwExtensionCount, glfwExtensions);

		if (this->ValidationLayersEnabled)
		{
			createInfo.enabledLayerCount = this->ValidationLayers.size();
			createInfo.ppEnabledLayerNames = this->ValidationLayers.data();
		}

		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::printf("%d extensions supported\n", extensionCount);

		this->VulkanInstance = vk::createInstance(createInfo);
	}

	template<std::size_t N>
	bool VulkanDeviceContext::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, std::span<const char* const, N> extensions)
	{
		auto availableExtensions = device.enumerateDeviceExtensionProperties();

		std::unordered_set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool VulkanDeviceContext::IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device)
	{
		QueueFamilyIndices indices;

		indices.FindQueueFamilies(device, this->Surface);

		auto extensionsSupported = CheckDeviceExtensionSupport(device, std::span{ this->DeviceExtentions });

		auto swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails details;
			QuerySwapChainSupport(device, this->Surface, details);
			swapChainAdequate = !details.Formats.empty() && !details.PresentModes.empty();
		}

		return indices.IsComplete() && extensionsSupported && swapChainAdequate;
	}

	void VulkanDeviceContext::PickPhysicalDevice()
	{
		auto physicalDevices = this->VulkanInstance.enumeratePhysicalDevices();

		for (const auto& device : physicalDevices)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				this->PhysicalDevice = device;
				this->PhysicalDeviceProperties = this->PhysicalDevice.getProperties();

				break;
			}
		}
	}

	void VulkanDeviceContext::CreateLogicalDevice()
	{
		QueueFamilyIndices indices;

		indices.FindQueueFamilies(this->PhysicalDevice, this->Surface);

		const std::unordered_set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentationFamily.value() };

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		for (const auto queueFamily : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo(
				{},
				queueFamily,
				1,
				&queuePriority);

			queueCreateInfos.push_back(queueCreateInfo);
		}

		vk::PhysicalDeviceFeatures deviceFeatures{};

		vk::DeviceCreateInfo createInfo(
			{},
			queueCreateInfos.size(),
			queueCreateInfos.data(),
			0, nullptr,
			this->DeviceExtentions.size(),
			this->DeviceExtentions.data(),
			&deviceFeatures);

		// This is here for compatibility with older implementations
		if (this->ValidationLayersEnabled)
		{
			createInfo.enabledLayerCount = this->ValidationLayers.size();
			createInfo.ppEnabledLayerNames = this->ValidationLayers.data();
		}

		// Create the logical device
		this->LogicalDevice = this->PhysicalDevice.createDevice(createInfo);

		// Get queues
		Queues.GetQueues(this->LogicalDevice, indices);
	}

	void VulkanDeviceContext::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices;
		queueFamilyIndices.FindQueueFamilies(this->PhysicalDevice, this->Surface);

		vk::CommandPoolCreateInfo poolInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			queueFamilyIndices.GraphicsFamily.value());

		this->CommandPool = this->LogicalDevice.createCommandPool(poolInfo);
	}

	void VulkanDeviceContext::CreateRenderSurface(GLFWwindow* Window)
	{
		vk::Win32SurfaceCreateInfoKHR createInfo(
			{},
			GetModuleHandle(nullptr),
			glfwGetWin32Window(Window));

		this->Surface = this->VulkanInstance.createWin32SurfaceKHR(createInfo);
	}
}