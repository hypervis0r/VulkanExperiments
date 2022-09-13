#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include <cstring>

#include "renderer/vulkanmem.h"

namespace Engine
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;

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

	class VertexBuffer
	{
	public:
		std::vector<Vertex> Vertices;
		vk::Buffer Buffer;
		vk::DeviceMemory BufferMemory;

		void Destroy(vk::Device& device)
		{
			VulkanMem::DestroyBuffer(device, this->Buffer, this->BufferMemory);
		}

		VertexBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const std::vector<Vertex>& verts)
			: Vertices(verts)
		{
			vk::DeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();
			VulkanMem::CreateBuffer(
				device,
				physicalDevice,
				this->Buffer,
				this->BufferMemory,
				bufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// Copy vertices to device memory
			auto data = device.mapMemory(this->BufferMemory, 0, bufferSize);
			std::memcpy(data, Vertices.data(), static_cast<size_t>(bufferSize));
			device.unmapMemory(this->BufferMemory);
		}

		constexpr VertexBuffer() 
			: Buffer(VK_NULL_HANDLE), BufferMemory(VK_NULL_HANDLE) {};
	};
}