#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstring>

#include "renderer/vulkanmem.h"

namespace Engine
{
	class Image
	{
	private:
		vk::Device& LogicalDevice;
		std::shared_ptr<VulkanMemManager> MemManager;

	public:
		vk::Image VulkanImage;
		vk::DeviceMemory ImageMemory;
		vk::ImageView ImageView;

		// TODO: Move sampler to separate class
		vk::Sampler Sampler;

		void CreateTextureImage(const char* texturePath);
		void CreateImageView(vk::Format format);
		void CreateTextureSampler();

		// Load from file
		Image(vk::Device& device, std::shared_ptr<VulkanMemManager> memManager, const char* texturePath) :
			LogicalDevice(device),
			MemManager(memManager)
		{
			CreateTextureImage(texturePath);
			CreateImageView(vk::Format::eR8G8B8A8Srgb);
			CreateTextureSampler();
		}

		// Image move
		Image(vk::Device& device, std::shared_ptr<VulkanMemManager> memManager,	vk::Image& image, vk::Format format) :
			LogicalDevice(device),
			MemManager(memManager)
		{
			CreateImageView(format);
			CreateTextureSampler();
		}
	};
}