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

	void VulkanMemManager::AllocMemory(const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags properties, vk::DeviceMemory& out)
	{
		vk::MemoryAllocateInfo allocInfo(
			memoryRequirements.size,
			FindMemoryType(this->PhysicalDevice, memoryRequirements.memoryTypeBits, properties));

		out = this->LogicalDevice.allocateMemory(allocInfo);
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

		AllocMemory(memRequirements, properties, bufferMemory);

		this->LogicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
	}

	void VulkanMemManager::BeginOneTimeCommands(vk::CommandBuffer& out)
	{
		out = this->LogicalDevice.allocateCommandBuffers(this->CommandBufferAllocInfo).front(); // TODO: This is retarded

		vk::CommandBufferBeginInfo beginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		out.begin(beginInfo);
	}

	void VulkanMemManager::EndOneTimeCommands(vk::CommandBuffer& commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// TODO: Maybe use fence here?
		this->Queues.GraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		this->Queues.GraphicsQueue.waitIdle();

		this->LogicalDevice.freeCommandBuffers(this->CommandPool, 1, &commandBuffer);
	}

	void VulkanMemManager::CopyBuffer(
		vk::Buffer& dst,
		vk::Buffer& src,
		vk::DeviceSize size)
	{
		vk::CommandBuffer commandBuffer;
		BeginOneTimeCommands(commandBuffer);

		vk::BufferCopy copyRegion(0, 0, size);
		commandBuffer.copyBuffer(src, dst, 1, &copyRegion);

		EndOneTimeCommands(commandBuffer);
	}

	void VulkanMemManager::CopyBufferToImage(
		vk::Image& dst, 
		vk::Buffer& src, 
		uint32_t width, 
		uint32_t height)
	{
		vk::CommandBuffer commandBuffer;
		BeginOneTimeCommands(commandBuffer);

		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = vk::Offset3D{ 0, 0, 0 };
		region.imageExtent = vk::Extent3D{ width, height, 1 };

		commandBuffer.copyBufferToImage(src, dst, vk::ImageLayout::eTransferDstOptimal, 1, &region);

		EndOneTimeCommands(commandBuffer);
	}

	void VulkanMemManager::TransitionImageLayout(
		vk::Image& image, 
		vk::Format format, 
		vk::ImageLayout oldLayout, 
		vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer;
		BeginOneTimeCommands(commandBuffer);

		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;

		// TODO: Find a more elegant way of doing this
		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			srcStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
			throw std::invalid_argument("Unsupported layout transition");

		commandBuffer.pipelineBarrier(
			srcStage,
			dstStage,
			{},
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndOneTimeCommands(commandBuffer);
	}

	void VulkanMemManager::DestroyImage(vk::Image& image, vk::DeviceMemory& imageMemory)
	{
		this->LogicalDevice.destroyImage(image);
		this->LogicalDevice.freeMemory(imageMemory);
	}

	void VulkanMemManager::CreateImage(
		vk::Image& image,
		vk::DeviceMemory& imageMemory,
		uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties)
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = this->LogicalDevice.createImage(imageInfo);

		auto memRequirements = this->LogicalDevice.getImageMemoryRequirements(image);

		AllocMemory(memRequirements, properties, imageMemory);

		this->LogicalDevice.bindImageMemory(image, imageMemory, 0);
	}
}