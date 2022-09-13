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
	bool Renderer::CheckVulkanLayerSupport(const std::span<const char* const, N> layers)
	{
		bool layerFound = false;
		auto availableLayers = vk::enumerateInstanceLayerProperties();

		for (const char* layerName : layers)
		{
			layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (std::strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	void Renderer::CreateVulkanInstance()
	{
		if (this->ValidationLayersEnabled && !CheckVulkanLayerSupport(std::span{ this->ValidationLayers }))
			throw std::runtime_error("Validation layers requested, but are not available.");

		vk::ApplicationInfo appInfo(
			"Hello Triangle",
			VK_MAKE_VERSION(1, 0, 0),
			"No Engine",
			VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0);

		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		if (glfwExtensions == nullptr || glfwExtensionCount == 0)
			throw std::runtime_error("Failed to get required GLFW instance extensions");

		vk::InstanceCreateInfo createInfo(
			vk::InstanceCreateFlags::Flags(),
			&appInfo,
			0, nullptr,
			glfwExtensionCount, glfwExtensions);

		if (this->ValidationLayersEnabled)
		{
			createInfo.enabledLayerCount = this->ValidationLayers.size();
			createInfo.ppEnabledLayerNames = this->ValidationLayers.data();
		}

		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::printf("%d extensions supported\n", extensionCount);

		this->VulkanInstance = vk::createInstance(createInfo);
	}

	template<std::size_t N>
	bool Renderer::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device, std::span<const char* const, N> extensions)
	{
		auto availableExtensions = device.enumerateDeviceExtensionProperties();

		std::unordered_set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool Renderer::IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device)
	{
		QueueFamilyIndices indices;
		
		indices.FindQueueFamilies(device, this->Surface);

		auto extensionsSupported = CheckDeviceExtensionSupport(device, std::span{ this->DeviceExtentions });

		auto swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails details;
			SwapChain::QuerySwapChainSupport(device, this->Surface, details);
			swapChainAdequate = !details.Formats.empty() && !details.PresentModes.empty();
		}

		return indices.IsComplete() && extensionsSupported && swapChainAdequate;
	}

	void Renderer::PickPhysicalDevice()
	{
		auto physicalDevices = this->VulkanInstance.enumeratePhysicalDevices();

		for (const auto& device : physicalDevices)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				this->PhysicalDevice = device;
				break;
			}
		}
	}

	void Renderer::CreateLogicalDevice()
	{
		QueueFamilyIndices indices;

		indices.FindQueueFamilies(this->PhysicalDevice, this->Surface);

		const std::unordered_set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentationFamily.value() };

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		for (const auto queueFamily : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo(
				vk::DeviceQueueCreateFlags::Flags(),
				queueFamily,
				1,
				&queuePriority);

			queueCreateInfos.push_back(queueCreateInfo);
		}

		vk::PhysicalDeviceFeatures deviceFeatures{};

		vk::DeviceCreateInfo createInfo(
			vk::DeviceCreateFlags::Flags(), 
			queueCreateInfos.size(),
			queueCreateInfos.data(),
			0, nullptr,
			this->DeviceExtentions.size(),
			this->DeviceExtentions.data(),
			&deviceFeatures);

		// This is here for compatibility with older implementations
		if (this->ValidationLayersEnabled)
		{
			createInfo.enabledLayerCount = this->ValidationLayers.size();
			createInfo.ppEnabledLayerNames = this->ValidationLayers.data();
		}

		// Create the logical device
		this->LogicalDevice = this->PhysicalDevice.createDevice(createInfo);

		// Get queues
		Queues.GetQueues(this->LogicalDevice, indices);
	}

	void Renderer::CreateRenderSurface()
	{
		vk::Win32SurfaceCreateInfoKHR createInfo(
			vk::Win32SurfaceCreateFlagsKHR::Flags(), 
			GetModuleHandle(nullptr), 
			glfwGetWin32Window(this->Window));

		this->Surface = this->VulkanInstance.createWin32SurfaceKHR(createInfo);
	}

	template<std::size_t N>
	void Renderer::CreateShaderModule(const std::span<char, N> code, vk::ShaderModule& module)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<uint32_t*>(code.data()); // TODO: I think this is wrong. Fuck you C++.

		module = this->LogicalDevice.createShaderModule(createInfo);
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
			vk::PipelineShaderStageCreateFlags::Flags(),
			vk::ShaderStageFlagBits::eVertex,
			vertShaderModule,
			"main");

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
			vk::PipelineShaderStageCreateFlags::Flags(),
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
			vk::PipelineDynamicStateCreateFlags::Flags(),
			dynamicStates.size(),
			dynamicStates.data());

		// Vertex input
		vk::VertexInputBindingDescription bindingDesc;
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescs;
		
		Vertex::GetBindingDescription(bindingDesc);
		Vertex::GetAttributeDescriptions(attributeDescs);

		const vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
			vk::PipelineVertexInputStateCreateFlags::Flags(),
			1, &bindingDesc,
			attributeDescs.size(), attributeDescs.data());

		// Input assembly
		constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
			vk::PipelineInputAssemblyStateCreateFlags::Flags(),
			vk::PrimitiveTopology::eTriangleList,
			VK_FALSE);

		// Viewport state
		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer(
			vk::PipelineRasterizationStateCreateFlags::Flags(),
			VK_FALSE,
			VK_FALSE,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eClockwise);
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
		this->PipelineLayout = this->LogicalDevice.createPipelineLayout(pipelineLayoutInfo);

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

		this->GraphicsPipeline = this->LogicalDevice.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;

		this->LogicalDevice.destroyShaderModule(vertShaderModule);
		this->LogicalDevice.destroyShaderModule(fragShaderModule);
	}

	void Renderer::CreateRenderPass()
	{
		const vk::AttachmentDescription colorAttachment(
			vk::AttachmentDescriptionFlags::Flags(),
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
			vk::AccessFlags::Flags(),
			vk::AccessFlagBits::eColorAttachmentWrite);

		vk::SubpassDescription subpass(
			vk::SubpassDescriptionFlags::Flags(),
			vk::PipelineBindPoint::eGraphics,
			0, nullptr,
			1, &colorAttachmentRef);

		// Create render pass
		vk::RenderPassCreateInfo renderPassInfo(
			vk::RenderPassCreateFlags::Flags(),
			1, &colorAttachment,
			1, &subpass,
			1, &dependency);

		this->RenderPass = this->LogicalDevice.createRenderPass(renderPassInfo);
	}

	void Renderer::CreateCommandBuffer()
	{
		vk::CommandBufferAllocateInfo allocInfo(
			this->CommandPool,
			vk::CommandBufferLevel::ePrimary,
			this->MAX_FRAMES_IN_FLIGHT);

		this->CommandBuffers = this->LogicalDevice.allocateCommandBuffers(allocInfo);
	}

	void Renderer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices;
		queueFamilyIndices.FindQueueFamilies(this->PhysicalDevice, this->Surface);

		vk::CommandPoolCreateInfo poolInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			queueFamilyIndices.GraphicsFamily.value());

		this->CommandPool = this->LogicalDevice.createCommandPool(poolInfo);
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

		commandBuffer.draw(vertexBuffer->Objects.size(), 1, 0, 0);

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
			this->ImageAvailableSemaphores.push_back(this->LogicalDevice.createSemaphore(semaphoreInfo));
			this->RenderFinishedSemaphores.push_back(this->LogicalDevice.createSemaphore(semaphoreInfo));
			this->InFlightFences.push_back(this->LogicalDevice.createFence(fenceInfo));
		}
	}

	void Renderer::InitializeVulkan()
	{
		CreateVulkanInstance();
		CreateRenderSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();

		this->Swapchain = SwapChain(this->PhysicalDevice, this->LogicalDevice, this->Window);

		this->Swapchain.CreateSwapChain(this->Surface);
		this->Swapchain.CreateImageViews();

		CreateRenderPass();
		CreateGraphicsPipeline();
		
		this->Swapchain.CreateFramebuffers(this->RenderPass);

		CreateCommandPool();

		this->MemManager = std::make_shared<VulkanMemManager>(this->LogicalDevice, this->PhysicalDevice, this->CommandPool, this->Queues);

		const std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};
		this->vertexBuffer = std::make_unique<VertexInputBuffer<Vertex>>(this->MemManager, vertices);

		CreateCommandBuffer();

		CreateSyncObjects();
	}

	void Renderer::Cleanup()
	{
		// Sync shit
		for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++)
		{
			this->LogicalDevice.destroySemaphore(this->ImageAvailableSemaphores[i]);
			this->LogicalDevice.destroySemaphore(this->RenderFinishedSemaphores[i]);
			this->LogicalDevice.destroyFence(this->InFlightFences[i]);
		}

		this->Swapchain.Destroy();

		this->vertexBuffer->Destroy();

		// Command pool
		this->LogicalDevice.destroyCommandPool(this->CommandPool);
		
		// Pipeline shit
		this->LogicalDevice.destroyPipeline(this->GraphicsPipeline);
		this->LogicalDevice.destroyPipelineLayout(this->PipelineLayout);
		this->LogicalDevice.destroyRenderPass(this->RenderPass);

		// Surface
		this->VulkanInstance.destroySurfaceKHR(this->Surface);

		// Device and instance
		this->LogicalDevice.destroy();
		this->VulkanInstance.destroy();

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

		this->LogicalDevice.waitIdle();
	}

	void Renderer::DrawFrame()
	{
		// Wait for previous frame to finish processing
		vk::resultCheck(this->LogicalDevice.waitForFences(1, &this->InFlightFences[this->CurrentFrame], VK_TRUE, UINT64_MAX), "Fence dumb shit");

		
		uint32_t imageIndex = 0;
		
		try
		{
			imageIndex = this->LogicalDevice.acquireNextImageKHR(
				this->Swapchain.Swapchain, 
				UINT64_MAX, 
				this->ImageAvailableSemaphores[this->CurrentFrame], 
				VK_NULL_HANDLE).value;
		}
		// If the swap chain is out of date (resized, etc.), we need to recreate it
		catch (vk::OutOfDateKHRError& ex)
		{
			this->Swapchain.RecreateSwapChain(this->Surface, this->RenderPass);
			return;
		}
		
		// Reset fence when we know we are operating on a frame.
		vk::resultCheck(this->LogicalDevice.resetFences(1, &this->InFlightFences[this->CurrentFrame]), "Fence dumb shit");

		this->CommandBuffers[this->CurrentFrame].reset();

		RecordCommandBuffer(this->CommandBuffers[this->CurrentFrame], imageIndex);

		const std::array<vk::Semaphore, 1> waitSemaphores = { this->ImageAvailableSemaphores[this->CurrentFrame] };
		const std::array<vk::Semaphore, 1> signalSemaphores = { this->RenderFinishedSemaphores[this->CurrentFrame] };
		constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		vk::SubmitInfo submitInfo(
			1, waitSemaphores.data(), waitStages.data(),
			1, &this->CommandBuffers[this->CurrentFrame],
			1, signalSemaphores.data());

		vk::resultCheck(this->Queues.GraphicsQueue.submit(1, &submitInfo, this->InFlightFences[this->CurrentFrame]),
			"Failed to submit command buffer.");

		const std::array<vk::SwapchainKHR, 1> swapChains = { this->Swapchain.Swapchain };

		vk::PresentInfoKHR presentInfo(
			1, signalSemaphores.data(),
			1, swapChains.data(),
			&imageIndex);

		// Present swap chain
		try
		{
			auto presentResult = this->Queues.PresentationQueue.presentKHR(presentInfo);

			// vulkan-hpp specifies eSuboptimalKHR as success, but whatever
			if (presentResult == vk::Result::eSuboptimalKHR)
				throw vk::OutOfDateKHRError("");
		}
		catch (vk::OutOfDateKHRError& ex)
		{
			this->Swapchain.RecreateSwapChain(this->Surface, this->RenderPass);
		};

		// Increment current frame so we can work on the next frame
		this->CurrentFrame = (this->CurrentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;
	}
}