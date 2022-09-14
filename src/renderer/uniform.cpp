#include "renderer/uniform.h"

namespace Engine
{
	void VulkanDescriptorPool::CreateDescriptorPool(uint32_t PoolSize)
	{
		vk::DescriptorPoolSize poolSize(
			vk::DescriptorType::eUniformBuffer,
			PoolSize);

		vk::DescriptorPoolCreateInfo poolInfo(
			{},
			PoolSize,
			1, &poolSize);

		this->DescriptorPool = this->LogicalDevice.createDescriptorPool(poolInfo);
	}

	void VulkanDescriptorPool::CreateDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding uboLayoutBinding(
			0,
			vk::DescriptorType::eUniformBuffer, 1,
			vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo layoutInfo(
			{}, 1, &uboLayoutBinding);

		this->DescriptorSetLayout = this->LogicalDevice.createDescriptorSetLayout(layoutInfo);
	}
}