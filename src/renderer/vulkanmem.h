#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>

namespace Engine
{
	class VulkanMemManager
	{
	private:
		vk::Device& LogicalDevice;
		vk::PhysicalDevice& PhysicalDevice;

		uint32_t FindMemoryType(vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	public:
		void* MapMemory(vk::DeviceMemory& memory, vk::DeviceSize offset, vk::DeviceSize size)
		{
			return this->LogicalDevice.mapMemory(memory, offset, size);
		}

		void UnmapMemory(vk::DeviceMemory& memory)
		{
			this->LogicalDevice.unmapMemory(memory);
		}

		void DestroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& deviceMemory);

		void CreateBuffer(
			vk::Buffer& buffer, 
			vk::DeviceMemory& bufferMemory, 
			vk::DeviceSize size, 
			vk::BufferUsageFlags usage, 
			vk::MemoryPropertyFlags properties);

		constexpr VulkanMemManager(vk::Device& device, vk::PhysicalDevice& physicalDevice) :
			LogicalDevice(device),
			PhysicalDevice(physicalDevice) {};

		VulkanMemManager() = delete;
	};
}