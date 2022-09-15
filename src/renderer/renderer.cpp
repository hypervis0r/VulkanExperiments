#include "renderer/renderer.h"

namespace Engine
{
	void Renderer::InitializeWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		this->Window = glfwCreateWindow(this->WindowWidth, this->WindowHeight, "VulkanExperiments", nullptr, nullptr);
		if (!this->Window)
			throw std::runtime_error("Failed to create window.");

		// TODO: Specify window resize callback so we can explicitly resize swap chain
	}

	template<std::size_t N>
	void Renderer::CreateShaderModule(const std::span<char, N> code, vk::ShaderModule& module)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<uint32_t*>(code.data()); // TODO: I think this is wrong. Fuck you C++.

		module = this->DeviceContext->LogicalDevice.createShaderModule(createInfo);
	}

	void Renderer::CreateGraphicsPipeline()
	{
		std::vector<char> vertShaderCode;
		std::vector<char> fragShaderCode;

		Filesystem::ReadFile("shaders/vert.spv", vertShaderCode);
		Filesystem::ReadFile("shaders/frag.spv", fragShaderCode);

		vk::ShaderModule vertShaderModule;
		vk::ShaderModule fragShaderModule;

		CreateShaderModule(std::span{ vertShaderCode }, vertShaderModule);
		CreateShaderModule(std::span{ fragShaderCode }, fragShaderModule);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
			{},
			vk::ShaderStageFlagBits::eVertex,
			vertShaderModule,
			"main");

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
			{},
			vk::ShaderStageFlagBits::eFragment,
			fragShaderModule,
			"main");

		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		// Dynamic states
		constexpr std::array<vk::DynamicState, 2> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		const vk::PipelineDynamicStateCreateInfo dynamicState(
			{},
			dynamicStates.size(),
			dynamicStates.data());

		// Vertex input
		vk::VertexInputBindingDescription bindingDesc;
		std::array<vk::VertexInputAttributeDescription, 3> attributeDescs;
		
		Vertex::GetBindingDescription(bindingDesc);
		Vertex::GetAttributeDescriptions(attributeDescs);

		const vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
			{},
			1, &bindingDesc,
			attributeDescs.size(), attributeDescs.data());

		// Input assembly
		constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
			{},
			vk::PrimitiveTopology::eTriangleList,
			VK_FALSE);

		// Viewport state
		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer(
			{},
			VK_FALSE,
			VK_FALSE,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		rasterizer.lineWidth = 1.0f;

		// Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling{};

		// Color blending
		// TODO: Maybe alpha blending
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional

		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Pipeline layout
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &this->DescriptorPool->DescriptorSetLayout;

		this->PipelineLayout = this->DeviceContext->LogicalDevice.createPipelineLayout(pipelineLayoutInfo);

		// Pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = this->PipelineLayout;
		pipelineInfo.renderPass = this->RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		this->GraphicsPipeline = this->DeviceContext->LogicalDevice.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;

		this->DeviceContext->LogicalDevice.destroyShaderModule(vertShaderModule);
		this->DeviceContext->LogicalDevice.destroyShaderModule(fragShaderModule);
	}

	void Renderer::CreateRenderPass()
	{
		const vk::AttachmentDescription colorAttachment(
			{},
			this->Swapchain.SwapChainImageFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, // TODO: Change to eLoad
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR // TODO: Change to eUndefined maybe
		);

		// Subpasses
		constexpr vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eAttachmentOptimal);

		vk::SubpassDependency dependency(
			VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite);

		vk::SubpassDescription subpass(
			{},
			vk::PipelineBindPoint::eGraphics,
			0, nullptr,
			1, &colorAttachmentRef);

		// Create render pass
		vk::RenderPassCreateInfo renderPassInfo(
			{},
			1, &colorAttachment,
			1, &subpass,
			1, &dependency);

		this->RenderPass = this->DeviceContext->LogicalDevice.createRenderPass(renderPassInfo);
	}

	void Renderer::CreateCommandBuffer()
	{
		vk::CommandBufferAllocateInfo allocInfo(
			this->DeviceContext->CommandPool,
			vk::CommandBufferLevel::ePrimary,
			this->MAX_FRAMES_IN_FLIGHT);

		this->CommandBuffers = this->DeviceContext->LogicalDevice.allocateCommandBuffers(allocInfo);
	}

	void Renderer::RecordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		vk::CommandBufferBeginInfo beginInfo{};
		commandBuffer.begin(beginInfo);

		constexpr std::array<float, 4> color = { 0.0f, 0.0f, 0.0f, 1.0f };

		vk::ClearValue clearColor{};
		clearColor.color = vk::ClearColorValue(color);

		vk::RenderPassBeginInfo renderPassInfo(
			this->RenderPass,
			this->Swapchain.SwapChainFramebuffers[imageIndex],
			vk::Rect2D{ {0, 0}, this->Swapchain.SwapChainExtent },
			1, &clearColor
		);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->GraphicsPipeline);

		// Viewport
		const vk::Viewport viewport(
			0.0f,
			0.0f,
			static_cast<float>(this->Swapchain.SwapChainExtent.width),
			static_cast<float>(this->Swapchain.SwapChainExtent.height),
			0.0f,
			1.0f);
		commandBuffer.setViewport(0, 1, &viewport);

		// Scissor
		const vk::Rect2D scissor(
			{ 0, 0 },
			this->Swapchain.SwapChainExtent);
		commandBuffer.setScissor(0, 1, &scissor);

		const std::array<vk::Buffer, 1> vertexBuffers = { this->vertexBuffer->Buffer };
		constexpr std::array<vk::DeviceSize, 1> offsets = { 0 };

		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
		commandBuffer.bindIndexBuffer(this->indexBuffer->Buffer, 0, Index::IndexType);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->PipelineLayout, 0, 1, &this->DescriptorPool->DescriptorSets[this->CurrentFrame], 0, nullptr);
		commandBuffer.drawIndexed(this->indexBuffer->Objects.size(), 1, 0, 0, 0);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void Renderer::CreateSyncObjects()
	{
		this->ImageAvailableSemaphores.reserve(this->MAX_FRAMES_IN_FLIGHT);
		this->RenderFinishedSemaphores.reserve(this->MAX_FRAMES_IN_FLIGHT);
		this->InFlightFences.reserve(this->MAX_FRAMES_IN_FLIGHT);
		
		vk::SemaphoreCreateInfo semaphoreInfo{};
		vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

		for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++)
		{
			this->ImageAvailableSemaphores.push_back(this->DeviceContext->LogicalDevice.createSemaphore(semaphoreInfo));
			this->RenderFinishedSemaphores.push_back(this->DeviceContext->LogicalDevice.createSemaphore(semaphoreInfo));
			this->InFlightFences.push_back(this->DeviceContext->LogicalDevice.createFence(fenceInfo));
		}
	}

	void Renderer::InitializeVulkan()
	{
		this->DeviceContext = std::make_shared<VulkanDeviceContext>(this->Window, this->ValidationLayersEnabled);

		this->Swapchain = SwapChain(this->DeviceContext, this->Window);

		this->Swapchain.CreateSwapChain(this->DeviceContext->Surface);
		this->Swapchain.CreateImageViews();

		CreateRenderPass();

		for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++)
		{
			Uniform<UniformBufferObject> uniform(*this->DeviceContext->MemManager);
			this->Uniforms.push_back(uniform);
		}

		this->Texture = std::make_unique<Image>(this->DeviceContext, "textures/queen.jpg");

		this->DescriptorPool = std::make_unique<VulkanDescriptorPool>(this->DeviceContext, this->MAX_FRAMES_IN_FLIGHT);
		this->DescriptorPool->CreateDescriptorSets(this->Uniforms, *this->Texture);

		CreateGraphicsPipeline();
		
		this->Swapchain.CreateFramebuffers(this->RenderPass);

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};
		this->vertexBuffer = std::make_unique<VertexInputBuffer<Vertex>>(this->DeviceContext, vertices);

		const std::vector<Index> indices = {
			0, 1, 2, 2, 3, 0
		};
		this->indexBuffer = std::make_unique<VertexInputBuffer<Index>>(this->DeviceContext, indices);

		CreateCommandBuffer();

		CreateSyncObjects();
	}

	void Renderer::Cleanup()
	{
		// Sync shit
		for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++)
		{
			this->DeviceContext->LogicalDevice.destroySemaphore(this->ImageAvailableSemaphores[i]);
			this->DeviceContext->LogicalDevice.destroySemaphore(this->RenderFinishedSemaphores[i]);
			this->DeviceContext->LogicalDevice.destroyFence(this->InFlightFences[i]);
		}

		this->Swapchain.Destroy();

		this->vertexBuffer->Destroy();
		this->indexBuffer->Destroy();

		for (auto& uniform : this->Uniforms)
		{
			uniform.Destroy(*this->DeviceContext->MemManager);
		}
		
		this->DescriptorPool->Destroy();

		this->Texture->Destroy();

		// Pipeline shit
		this->DeviceContext->LogicalDevice.destroyPipeline(this->GraphicsPipeline);
		this->DeviceContext->LogicalDevice.destroyPipelineLayout(this->PipelineLayout);
		this->DeviceContext->LogicalDevice.destroyRenderPass(this->RenderPass);

		// Window
		glfwDestroyWindow(this->Window);
		this->Window = nullptr;

		// GLFW
		glfwTerminate();
	}

	void Renderer::MainRenderLoop()
	{
		while (!glfwWindowShouldClose(this->Window))
		{
			glfwPollEvents();
			DrawFrame();
		}

		this->DeviceContext->LogicalDevice.waitIdle();
	}
	
	void Renderer::UpdateUniformWithNewData(Uniform<UniformBufferObject> Uniform)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(
			glm::radians(45.0f), 
			this->Swapchain.SwapChainExtent.width / static_cast<float>(this->Swapchain.SwapChainExtent.height), 
			0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		Uniform.UpdateUniformBuffer(*this->DeviceContext->MemManager, ubo);
	}

	void Renderer::DrawFrame()
	{
		// Wait for previous frame to finish processing
		vk::resultCheck(this->DeviceContext->LogicalDevice.waitForFences(1, &this->InFlightFences[this->CurrentFrame], VK_TRUE, UINT64_MAX), "Fence dumb shit");

		
		uint32_t imageIndex = 0;
		
		try
		{
			imageIndex = this->DeviceContext->LogicalDevice.acquireNextImageKHR(
				this->Swapchain.Swapchain, 
				UINT64_MAX, 
				this->ImageAvailableSemaphores[this->CurrentFrame], 
				VK_NULL_HANDLE).value;
		}
		// If the swap chain is out of date (resized, etc.), we need to recreate it
		catch (vk::OutOfDateKHRError&)
		{
			this->Swapchain.RecreateSwapChain(this->DeviceContext->Surface, this->RenderPass);
			return;
		}
		
		// Reset fence when we know we are operating on a frame.
		vk::resultCheck(this->DeviceContext->LogicalDevice.resetFences(1, &this->InFlightFences[this->CurrentFrame]), "Fence dumb shit");

		this->CommandBuffers[this->CurrentFrame].reset();

		RecordCommandBuffer(this->CommandBuffers[this->CurrentFrame], imageIndex);

		const std::array<vk::Semaphore, 1> waitSemaphores = { this->ImageAvailableSemaphores[this->CurrentFrame] };
		const std::array<vk::Semaphore, 1> signalSemaphores = { this->RenderFinishedSemaphores[this->CurrentFrame] };
		constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		// Update shader uniforms
		UpdateUniformWithNewData(this->Uniforms[this->CurrentFrame]);

		vk::SubmitInfo submitInfo(
			1, waitSemaphores.data(), waitStages.data(),
			1, &this->CommandBuffers[this->CurrentFrame],
			1, signalSemaphores.data());

		vk::resultCheck(this->DeviceContext->Queues.GraphicsQueue.submit(1, &submitInfo, this->InFlightFences[this->CurrentFrame]),
			"Failed to submit command buffer.");

		const std::array<vk::SwapchainKHR, 1> swapChains = { this->Swapchain.Swapchain };

		vk::PresentInfoKHR presentInfo(
			1, signalSemaphores.data(),
			1, swapChains.data(),
			&imageIndex);

		// Present swap chain
		try
		{
			auto presentResult = this->DeviceContext->Queues.PresentationQueue.presentKHR(presentInfo);

			// vulkan-hpp specifies eSuboptimalKHR as success, but whatever
			if (presentResult == vk::Result::eSuboptimalKHR)
				throw vk::OutOfDateKHRError("");
		}
		catch (vk::OutOfDateKHRError&)
		{
			this->Swapchain.RecreateSwapChain(this->DeviceContext->Surface, this->RenderPass);
		};

		// Increment current frame so we can work on the next frame
		this->CurrentFrame = (this->CurrentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;
	}
}