#include "renderer/uniform.h"

namespace Engine
{
	void VulkanDescriptorPool::CreateDescriptorPool(uint32_t PoolSize)
	{
		std::array<vk::DescriptorPoolSize, 2> poolSizes{};

		poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
		poolSizes[0].descriptorCount = PoolSize;
		poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
		poolSizes[1].descriptorCount = PoolSize;

		vk::DescriptorPoolCreateInfo poolInfo(
			{},
			PoolSize,
			static_cast<uint32_t>(poolSizes.size()), 
			poolSizes.data());

		this->DescriptorPool = this->LogicalDevice.createDescriptorPool(poolInfo);
	}

	void VulkanDescriptorPool::CreateDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding uboLayoutBinding(
			0,
			vk::DescriptorType::eUniformBuffer, 1,
			vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutBinding samplerLayoutBinding(
			1,
			vk::DescriptorType::eCombinedImageSampler, 1,
			vk::ShaderStageFlagBits::eFragment);

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		vk::DescriptorSetLayoutCreateInfo layoutInfo(
			{}, 
			static_cast<uint32_t>(bindings.size()), 
			bindings.data());

		this->DescriptorSetLayout = this->LogicalDevice.createDescriptorSetLayout(layoutInfo);
	}
}