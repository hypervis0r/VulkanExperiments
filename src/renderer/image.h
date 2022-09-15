#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstring>

#include "renderer/vulkandevicecontext.h"
#include "renderer/vulkanmem.h"

namespace Engine
{
	class Image
	{
	private:
		std::shared_ptr<VulkanDeviceContext> DeviceContext;

	public:
		vk::Image VulkanImage;
		vk::DeviceMemory ImageMemory;
		vk::ImageView ImageView;

		// TODO: Move sampler to separate class
		vk::Sampler Sampler;

		void CreateTextureImage(const char* texturePath);
		void CreateImageView(vk::Format format);
		void CreateTextureSampler();

		void DestroyImageView()
		{
			this->DeviceContext->LogicalDevice.destroyImageView(this->ImageView);
		}

		void Destroy()
		{
			this->DeviceContext->LogicalDevice.destroySampler(this->Sampler);

			DestroyImageView();

			this->DeviceContext->MemManager->DestroyImage(this->VulkanImage, this->ImageMemory);
		}

		// Load from file
		Image(std::shared_ptr<VulkanDeviceContext> devCtx, const char* texturePath) :
			DeviceContext(devCtx)
		{
			CreateTextureImage(texturePath);
			CreateImageView(vk::Format::eR8G8B8A8Srgb);
			CreateTextureSampler();
		}

		// Image move
		Image(std::shared_ptr<VulkanDeviceContext> devCtx, vk::Image& image, vk::Format format) :
			DeviceContext(devCtx)
		{
			CreateImageView(format);
			CreateTextureSampler();
		}
	};
}