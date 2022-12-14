#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include <cstring>
#include <memory>

#include "renderer/vulkandevicecontext.h"
#include "renderer/vulkanmem.h"

namespace Engine
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		constexpr static vk::BufferUsageFlagBits BufferType = vk::BufferUsageFlagBits::eVertexBuffer;

		constexpr static void GetBindingDescription(vk::VertexInputBindingDescription& bindingDescription)
		{
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		}

		constexpr static void GetAttributeDescriptions(std::array<vk::VertexInputAttributeDescription, 3>& attributeDescriptions)
		{
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat; // vec3
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat; // vec3
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR32G32Sfloat; // vec2
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
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
		std::shared_ptr<VulkanDeviceContext> DeviceContext;

	public:
		std::vector<T> Objects;
		vk::Buffer Buffer;
		vk::DeviceMemory BufferMemory;

		void Destroy()
		{
			this->DeviceContext->MemManager->DestroyBuffer(this->Buffer, this->BufferMemory);
		}

		VertexInputBuffer(std::shared_ptr<VulkanDeviceContext> devCtx, const std::vector<T>& verts)
			: Objects(verts), DeviceContext(devCtx)
		{
			const vk::DeviceSize bufferSize = sizeof(Objects[0]) * Objects.size();

			// Create staging buffer
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			this->DeviceContext->MemManager->CreateBuffer(
				stagingBuffer,
				stagingBufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// Copy vertices to staging buffer
			auto data = this->DeviceContext->MemManager->MapMemory(stagingBufferMemory, 0, bufferSize);
			std::memcpy(data, Objects.data(), static_cast<size_t>(bufferSize));
			this->DeviceContext->MemManager->UnmapMemory(stagingBufferMemory);

			this->DeviceContext->MemManager->CreateBuffer(
				this->Buffer,
				this->BufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | T::BufferType,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			this->DeviceContext->MemManager->CopyBuffer(this->Buffer, stagingBuffer, bufferSize);

			this->DeviceContext->MemManager->DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}
	};
}