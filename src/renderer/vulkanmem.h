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

		// TODO: SET ME FREE
		vk::CommandBufferAllocateInfo CommandBufferAllocInfo;

		uint32_t FindMemoryType(vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		void AllocMemory(const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags properties, vk::DeviceMemory& out);

		// TODO: THESE DONT BELONG HERE
		void BeginOneTimeCommands(vk::CommandBuffer& out);
		void EndOneTimeCommands(vk::CommandBuffer& commandBuffer);

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

		void CopyBufferToImage(
			vk::Image& dst, 
			vk::Buffer& src, 
			uint32_t width, 
			uint32_t height);

		void DestroyImage(vk::Image& image, vk::DeviceMemory& imageMemory);

		void CreateImage(
			vk::Image& image,
			vk::DeviceMemory& imageMemory,
			uint32_t width,
			uint32_t height, 
			vk::Format format, 
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties);

		void TransitionImageLayout(
			vk::Image& image, 
			vk::Format format, 
			vk::ImageLayout oldLayout, 
			vk::ImageLayout newLayout);

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