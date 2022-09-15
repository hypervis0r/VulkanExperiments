#include "renderer/image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine
{
	void Image::CreateImageView(vk::Format format)
	{
		vk::ImageViewCreateInfo createInfo{};

		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = format;

		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;

		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		createInfo.image = this->VulkanImage;

		this->ImageView = this->DeviceContext->LogicalDevice.createImageView(createInfo);
	}

	void Image::CreateTextureImage(const char* texturePath)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		
		if (!pixels)
			throw std::runtime_error("Failed to load texture image.");

		vk::DeviceSize imageSize = texWidth * texHeight * 4;
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		this->DeviceContext->MemManager->CreateBuffer(
			stagingBuffer,
			stagingBufferMemory,
			imageSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto data = this->DeviceContext->MemManager->MapMemory(stagingBufferMemory, 0, imageSize);
		std::memcpy(data, pixels, static_cast<size_t>(imageSize));
		this->DeviceContext->MemManager->UnmapMemory(stagingBufferMemory);

		stbi_image_free(pixels);

		this->DeviceContext->MemManager->CreateImage(
			this->VulkanImage,
			this->ImageMemory,
			texWidth,
			texHeight,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		this->DeviceContext->MemManager->TransitionImageLayout(
			this->VulkanImage,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal);

		this->DeviceContext->MemManager->CopyBufferToImage(this->VulkanImage, stagingBuffer, texWidth, texHeight);

		this->DeviceContext->MemManager->TransitionImageLayout(
			this->VulkanImage, 
			vk::Format::eR8G8B8A8Srgb, 
			vk::ImageLayout::eTransferDstOptimal, 
			vk::ImageLayout::eShaderReadOnlyOptimal);

		this->DeviceContext->MemManager->DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}

	void Image::CreateTextureSampler()
	{
		vk::SamplerCreateInfo samplerInfo{};

		// TODO: Add configuration
		samplerInfo.magFilter = samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.anisotropyEnable = VK_FALSE; // TODO: Enable anisotropy
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = vk::CompareOp::eAlways;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		this->Sampler = this->DeviceContext->LogicalDevice.createSampler(samplerInfo);
	}
}