#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

#include "renderer/vulkanmem.h"
#include "renderer/image.h"

namespace Engine
{
	template <typename T>
	struct Uniform
	{
		vk::Buffer UniformBuffer;
		vk::DeviceMemory UniformBufferMemory;
		const size_t UniformSize = sizeof(T);

		template <typename T>
		void UpdateUniformBuffer(VulkanMemManager& MemManager, T& object)
		{
			auto data = MemManager.MapMemory(this->UniformBufferMemory, 0, this->UniformSize);
			std::memcpy(data, &object, this->UniformSize);
			MemManager.UnmapMemory(this->UniformBufferMemory);
		}

		void CreateUniformBuffer(VulkanMemManager& MemManager)
		{
			vk::DeviceSize bufferSize = this->UniformSize;

			MemManager.CreateBuffer(
				this->UniformBuffer,
				this->UniformBufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}

		void Destroy(VulkanMemManager& MemManager)
		{
			MemManager.DestroyBuffer(this->UniformBuffer, this->UniformBufferMemory);
		}

		Uniform<T>(VulkanMemManager& MemManager)
		{
			CreateUniformBuffer(MemManager);
		}
	};

	class VulkanDescriptorPool
	{
	private:
		vk::DescriptorPool DescriptorPool;
		vk::Device& LogicalDevice;
		uint32_t PoolSize;
		std::shared_ptr<VulkanMemManager> MemManager;
		
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool(uint32_t PoolSize);


	public:
		vk::DescriptorSetLayout DescriptorSetLayout;
		std::vector<vk::DescriptorSet> DescriptorSets;

		template <typename T>
		void CreateDescriptorSets(std::vector<Uniform<T>>& uniforms, Image texture)
		{
			std::vector<vk::DescriptorSetLayout> layouts(this->PoolSize, this->DescriptorSetLayout);

			vk::DescriptorSetAllocateInfo allocInfo(
				this->DescriptorPool, layouts);

			this->DescriptorSets = this->LogicalDevice.allocateDescriptorSets(allocInfo);

			for (size_t i = 0; i < this->PoolSize; i++)
			{
				vk::DescriptorBufferInfo bufferInfo(
					uniforms[i].UniformBuffer,
					0,
					uniforms[i].UniformSize);

				// TODO: Make this better
				vk::DescriptorImageInfo imageInfo(
					texture.Sampler, 
					texture.ImageView, 
					vk::ImageLayout::eShaderReadOnlyOptimal);

				std::array<vk::WriteDescriptorSet, 2> descWrites{};

				descWrites[0].dstSet = this->DescriptorSets[i];
				descWrites[0].dstBinding = 0;
				descWrites[0].dstArrayElement = 0;
				descWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
				descWrites[0].descriptorCount = 1;
				descWrites[0].pBufferInfo = &bufferInfo;

				descWrites[1].dstSet = this->DescriptorSets[i];
				descWrites[1].dstBinding = 1;
				descWrites[1].dstArrayElement = 0;
				descWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
				descWrites[1].descriptorCount = 1;
				descWrites[1].pImageInfo = &imageInfo;

				this->LogicalDevice.updateDescriptorSets(descWrites, nullptr);
			}
		}

		VulkanDescriptorPool(vk::Device& device, std::shared_ptr<VulkanMemManager> memManager, uint32_t size) :
			LogicalDevice(device),
			MemManager(memManager),
			PoolSize(size)
		{
			CreateDescriptorPool(this->PoolSize);
			CreateDescriptorSetLayout();
		}

		void Destroy()
		{
			this->LogicalDevice.destroyDescriptorPool(this->DescriptorPool);
			this->LogicalDevice.destroyDescriptorSetLayout(this->DescriptorSetLayout);
		}
	};
}