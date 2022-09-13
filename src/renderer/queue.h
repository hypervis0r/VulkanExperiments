#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <optional>
#include <cstdint>

namespace Engine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentationFamily;

		bool IsComplete()
		{
			return GraphicsFamily.has_value()
				&& PresentationFamily.has_value();
		}

		void FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
		{
			auto queueFamilies = device.getQueueFamilyProperties();

			int i = 0;
			for (const auto& family : queueFamilies)
			{
				if (family.queueFlags & vk::QueueFlagBits::eGraphics)
					this->GraphicsFamily = i;

				if (device.getSurfaceSupportKHR(i, surface))
					this->PresentationFamily = i;

				if (this->IsComplete())
					break;

				++i;
			}
		}
	};

	struct VulkanQueues
	{
		vk::Queue GraphicsQueue;
		vk::Queue PresentationQueue;

		void GetQueues(vk::Device& device, const QueueFamilyIndices& indices)
		{
			// Create the command queues
			this->GraphicsQueue = device.getQueue(indices.GraphicsFamily.value(), 0);
			this->PresentationQueue = device.getQueue(indices.PresentationFamily.value(), 0);
		}

		constexpr VulkanQueues() :
			GraphicsQueue(VK_NULL_HANDLE),
			PresentationQueue(VK_NULL_HANDLE) {}
	};
}