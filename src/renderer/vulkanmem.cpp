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
			{},
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

	void VulkanMemManager::CopyBuffer(
		vk::Buffer& dst,
		vk::Buffer& src,
		vk::DeviceSize size)
	{
		vk::CommandBuffer commandBuffer;
		commandBuffer = this->LogicalDevice.allocateCommandBuffers(this->CommandBufferAllocInfo).front(); // TODO: This is retarded

		vk::CommandBufferBeginInfo beginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		commandBuffer.begin(beginInfo);

		vk::BufferCopy copyRegion(0, 0, size);
		commandBuffer.copyBuffer(src, dst, 1, &copyRegion);

		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// TODO: Maybe use fence here?
		this->Queues.GraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		this->Queues.GraphicsQueue.waitIdle();

		this->LogicalDevice.freeCommandBuffers(this->CommandPool, 1, &commandBuffer);
	}
}