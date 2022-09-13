#include "renderer/vulkanmem.h"

namespace Engine
{
	void VulkanMem::CreateBuffer(
		vk::Device& device,
		vk::PhysicalDevice& physicalDevice,
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

		buffer = device.createBuffer(bufferInfo);

		auto memRequirements = device.getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo allocInfo(
			memRequirements.size, 
			FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties));

		bufferMemory = device.allocateMemory(allocInfo);

		device.bindBufferMemory(buffer, bufferMemory, 0);
	}
}