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
}