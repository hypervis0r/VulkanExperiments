#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>

namespace Engine
{
	static class VulkanMem
	{
	private:
		static uint32_t FindMemoryType(vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			auto memProperties = device.getMemoryProperties();

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && 
					((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type.");
		}

	public:
		static void DestroyBuffer(vk::Device& device, vk::Buffer& buffer, vk::DeviceMemory& deviceMemory)
		{
			device.destroyBuffer(buffer);
			device.freeMemory(deviceMemory);
		}

		static void CreateBuffer(
			vk::Device& device,
			vk::PhysicalDevice& physicalDevice,
			vk::Buffer& buffer, 
			vk::DeviceMemory& bufferMemory, 
			vk::DeviceSize size, 
			vk::BufferUsageFlags usage, 
			vk::MemoryPropertyFlags properties);
	};
}