#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include <cstring>
#include <memory>

#include "renderer/vulkanmem.h"

namespace Engine
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;

		constexpr static vk::BufferUsageFlagBits BufferType = vk::BufferUsageFlagBits::eVertexBuffer;

		constexpr static void GetBindingDescription(vk::VertexInputBindingDescription& bindingDescription)
		{
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		}

		constexpr static void GetAttributeDescriptions(std::array<vk::VertexInputAttributeDescription, 2>& attributeDescriptions)
		{
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat; // vec3
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat; // vec3
			attributeDescriptions[1].offset = offsetof(Vertex, color);
		}
	};

	struct Index
	{
		uint32_t index;

		constexpr static vk::BufferUsageFlagBits BufferType = vk::BufferUsageFlagBits::eIndexBuffer;
		constexpr static vk::IndexType IndexType = vk::IndexType::eUint32;

		constexpr Index(uint32_t idx) : index(idx) {}
	};

	template<typename T>
	class VertexInputBuffer
	{
	private:
		std::shared_ptr<VulkanMemManager> MemManager;

	public:
		std::vector<T> Objects;
		vk::Buffer Buffer;
		vk::DeviceMemory BufferMemory;

		void Destroy()
		{
			MemManager->DestroyBuffer(this->Buffer, this->BufferMemory);
		}

		VertexInputBuffer(std::shared_ptr<VulkanMemManager> manager, const std::vector<T>& verts)
			: Objects(verts), MemManager(manager)
		{
			const vk::DeviceSize bufferSize = sizeof(Objects[0]) * Objects.size();

			// Create staging buffer
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			this->MemManager->CreateBuffer(
				stagingBuffer,
				stagingBufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// Copy vertices to staging buffer
			auto data = this->MemManager->MapMemory(stagingBufferMemory, 0, bufferSize);
			std::memcpy(data, Objects.data(), static_cast<size_t>(bufferSize));
			this->MemManager->UnmapMemory(stagingBufferMemory);

			this->MemManager->CreateBuffer(
				this->Buffer,
				this->BufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | T::BufferType,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			this->MemManager->CopyBuffer(this->Buffer, stagingBuffer, bufferSize);

			this->MemManager->DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}
	};
}