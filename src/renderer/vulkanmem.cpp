#include "renderer/vulkanmem.h"

namespace Engine
{
	uint32_t VulkanMemManager::FindMemoryType(vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
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

	void VulkanMemManager::DestroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& deviceMemory)
	{
		this->LogicalDevice.destroyBuffer(buffer);
		this->LogicalDevice.freeMemory(deviceMemory);
	}

	void VulkanMemManager::CreateBuffer(
		vk::Buffer& buffer,
		vk::DeviceMemory& bufferMemory,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties)
	{
		vk::BufferCreateInfo bufferInfo(
			vk::BufferCreateFlags::Flags(),
			size,
			usage);

		buffer = this->LogicalDevice.createBuffer(bufferInfo);

		auto memRequirements = this->LogicalDevice.getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size, 
			FindMemoryType(this->PhysicalDevice, memRequirements.memoryTypeBits, properties));

		bufferMemory = this->LogicalDevice.allocateMemory(allocInfo);

		this->LogicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
	}
}