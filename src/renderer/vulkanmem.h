#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>

#include "renderer/queue.h"

namespace Engine
{
	class VulkanMemManager
	{
	private:
		vk::Device& LogicalDevice;
		vk::PhysicalDevice& PhysicalDevice;
		vk::CommandPool& CommandPool;
		VulkanQueues& Queues;

		vk::CommandBufferAllocateInfo CommandBufferAllocInfo;

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

		void CopyBuffer(
			vk::Buffer& dst,
			vk::Buffer& src,
			vk::DeviceSize size);

		VulkanMemManager() = delete;

		constexpr VulkanMemManager(
			vk::Device& device, 
			vk::PhysicalDevice& physicalDevice, 
			vk::CommandPool& commandPool,
			VulkanQueues& queues) : 
		LogicalDevice(device),
		PhysicalDevice(physicalDevice),
		CommandPool(commandPool),
		Queues(queues) 
		{
			this->CommandBufferAllocInfo.level = vk::CommandBufferLevel::ePrimary;
			this->CommandBufferAllocInfo.commandPool = CommandPool;
			this->CommandBufferAllocInfo.commandBufferCount = 1;
		};
	};
}