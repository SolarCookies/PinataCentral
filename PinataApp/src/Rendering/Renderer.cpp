#include "Renderer.h"
#include <array>
#include <iostream>

#include "../Utils/OpenFileDialog.h"

#include "../GlobalSettings.h"

#include "glm/gtc/type_ptr.hpp"

#include "Walnut/Application.h"
//#include "imgui.h"
//#include "backends/imgui_impl_vulkan.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

namespace cubed {



	static uint32_t ImGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t typeBits)
	{
		VkPhysicalDevice physicalDevice = cubed::GetVulkanInfo()->PhysicalDevice;

		VkPhysicalDeviceMemoryProperties prop;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &prop);
		for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
			if ((prop.memoryTypes[i].propertyFlags & properties) == properties && (typeBits & (1 << i)))
				return i;
		return 0xFFFFFFFF; // Unable to find memoryType

	}

	void Renderer::Init(ImGui_ImplVulkanH_Window* mainWindow) {
		std::cout << "Renderer initializing" << std::endl;
		InitBuffers();
		InitPipeline(mainWindow);
		CreateOffscreenFramebuffer(mainWindow);
		

	};

	void Renderer::Shutdown() {
		std::cout << "Renderer shutting down" << std::endl;

		VkDevice device = cubed::GetVulkanInfo()->Device;
		
		if (m_OffscreenFramebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device, m_OffscreenFramebuffer, nullptr);
			m_OffscreenFramebuffer = VK_NULL_HANDLE;
		}

		if (m_OffscreenRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, m_OffscreenRenderPass, nullptr);
			m_OffscreenRenderPass = VK_NULL_HANDLE;
		}

		if (m_OffscreenImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, m_OffscreenImageView, nullptr);
			m_OffscreenImageView = VK_NULL_HANDLE;
		}

		if (m_OffscreenImage != VK_NULL_HANDLE) {
			vkDestroyImage(device, m_OffscreenImage, nullptr);
			m_OffscreenImage = VK_NULL_HANDLE;
		}

		if (m_OffscreenImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, m_OffscreenImageMemory, nullptr);
			m_OffscreenImageMemory = VK_NULL_HANDLE;
		}
		
		if (m_GraphicsPipline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, m_GraphicsPipline, nullptr);
			m_GraphicsPipline = VK_NULL_HANDLE;
		}

		if (m_PipelineLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
			m_PipelineLayout = VK_NULL_HANDLE;
		}

		if (m_VertexBuffer.Handle != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, m_VertexBuffer.Handle, nullptr);
			m_VertexBuffer.Handle = VK_NULL_HANDLE;
		}

		if (m_VertexBuffer.BufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, m_VertexBuffer.BufferMemory, nullptr);
			m_VertexBuffer.BufferMemory = VK_NULL_HANDLE;
		}

		if (m_IndexBuffer.Handle != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, m_IndexBuffer.Handle, nullptr);
			m_IndexBuffer.Handle = VK_NULL_HANDLE;
		}

		if (m_IndexBuffer.BufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, m_IndexBuffer.BufferMemory, nullptr);
			m_IndexBuffer.BufferMemory = VK_NULL_HANDLE;
		}

		std::cout << "Renderer shutdown complete" << std::endl;
	};

	void Renderer::Render(ImGui_ImplVulkanH_Window* mainWindow, int Width, int Height) {
		VkCommandBuffer commandBuffer = Walnut::Application::GetActiveCommandBuffer();
		if (commandBuffer == VK_NULL_HANDLE) {
			std::cerr << "Invalid command buffer" << std::endl;
			return;
		}

		// Begin command buffer
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		// Begin render pass for offscreen framebuffer
		if (m_OffscreenFramebuffer == VK_NULL_HANDLE) {
			std::cerr << "Invalid offscreen framebuffer" << std::endl;
			return;
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_OffscreenRenderPass;
		renderPassInfo.framebuffer = m_OffscreenFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { uint32_t(mainWindow->Width), uint32_t(mainWindow->Height) };

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind the graphics pipeline.
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipline);

		// Clamp width to max window width
		int FixedWidth = glm::clamp(Width, 0, mainWindow->Width);
		int FixedHeight = glm::clamp(Height, 0, mainWindow->Height);

		glm::mat4 cameraTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 3.0f));

		m_PushConstants.ViewProjection = glm::perspectiveFov(glm::radians(45.0f), (float)FixedWidth, (float)FixedHeight, 0.1f, 1000.0f)
			* glm::inverse(cameraTransform);

		m_PushConstants.Transform = glm::translate(glm::mat4(1.0f), m_CubePosition) * glm::eulerAngleXYZ(glm::radians(m_CubeRotation.x), glm::radians(m_CubeRotation.y), glm::radians(m_CubeRotation.z));

		vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m_PushConstants), &m_PushConstants);

		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer.Handle, &offset);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.Handle, offset, VK_INDEX_TYPE_UINT32);

		VkViewport vp{};
		vp.width = static_cast<float>(FixedWidth);
		vp.height = static_cast<float>(FixedHeight);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &vp);

		VkRect2D scissor{};
		scissor.extent.width = FixedWidth;
		scissor.extent.height = FixedHeight;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		// End command buffer
		vkEndCommandBuffer(commandBuffer);

		std::cout << "Render Complete" << std::endl;
	}


	void Renderer::RenderUI()
	{
		ImGui::Begin("Controls");
		ImGui::DragFloat3("Position", glm::value_ptr(m_CubePosition));
		ImGui::DragFloat3("Rotation", glm::value_ptr(m_CubeRotation));

		//add a button to test popup
		if (ImGui::Button("Test Popup"))
		{
			std::cout << "Popup test" << std::endl;
			ShowPopup(std::string("Test popup"), true);
			UpdateProgressBar(0.5f);
		}

		ImGui::End();

		
	}

	VkShaderModule Renderer::LoadShader(const std::filesystem::path& path)
	{
		std::ifstream stream(path, std::ios::binary);
		if (!stream.is_open())
		{
			//Print could not open file
			std::cout << "Could not open file!: " << path.string() << std::endl;
			return nullptr;
		}
		std::cout << "Loaded file: " << path.string() << std::endl;
		stream.seekg(0, std::ios_base::end);
		std::streampos size = stream.tellg();
		stream.seekg(0, std::ios_base::beg);

		std::vector<char> buffer(size);
		stream.read(buffer.data(), size);

		stream.close();

		VkShaderModuleCreateInfo shaderModuleCI{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

		shaderModuleCI.pCode = (uint32_t*)buffer.data();
		shaderModuleCI.codeSize = buffer.size();

		VkDevice device = cubed::GetVulkanInfo()->Device;
		VkShaderModule result;
		VK_CHECK(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &result));

		return result;
	}

	void Renderer::InitPipeline(ImGui_ImplVulkanH_Window* mainWindow) {

		VkDevice device = cubed::GetVulkanInfo()->Device;

		VkRenderPass renderPass = mainWindow->RenderPass;

		// Create a blank pipeline layout.
	// We are not binding any resources to the pipeline in this first sample.
		VkPipelineLayoutCreateInfo layout_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

		std::array<VkPushConstantRange, 1> push_constants{};
		push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constants[0].offset = 0;
		push_constants[0].size = sizeof(glm::vec4) * 2;

		layout_info.pPushConstantRanges = push_constants.data();
		layout_info.pushConstantRangeCount = (uint32_t)push_constants.size();


		VK_CHECK(vkCreatePipelineLayout(device, &layout_info, nullptr, &m_PipelineLayout));

		VkVertexInputBindingDescription binding_desc[1]{};
		binding_desc[0].stride = sizeof(glm::vec3);
		binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attribute_desc[1]{};
		attribute_desc[0].location = 0;
		attribute_desc[0].binding = binding_desc[0].binding;
		attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_desc[0].offset = 0;


		VkPipelineVertexInputStateCreateInfo vertex_input{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertex_input.vertexBindingDescriptionCount = 1;
		vertex_input.pVertexBindingDescriptions = binding_desc;
		vertex_input.vertexAttributeDescriptionCount = 1;
		vertex_input.pVertexAttributeDescriptions = attribute_desc;

		// Specify we will use triangle lists to draw geometry.
		VkPipelineInputAssemblyStateCreateInfo input_assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Specify rasterization state.
		VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		//raster.cullMode = VK_CULL_MODE_NONE;
		raster.cullMode = VK_CULL_MODE_BACK_BIT;
		raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster.lineWidth = 1.0f;

		// Our attachment will write to all color channels, but no blending is enabled.
		VkPipelineColorBlendAttachmentState blend_attachment{};
		blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.attachmentCount = 1;
		blend.pAttachments = &blend_attachment;

		// We will have one viewport and scissor box.
		VkPipelineViewportStateCreateInfo viewport{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewport.viewportCount = 1;
		viewport.scissorCount = 1;

		// Disable all depth testing.
		VkPipelineDepthStencilStateCreateInfo depth_stencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		// No multisampling.
		VkPipelineMultisampleStateCreateInfo multisample{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Specify that these states will be dynamic, i.e. not part of pipeline state object.
		std::array<VkDynamicState, 2> dynamics{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamic.pDynamicStates = dynamics.data();
		dynamic.dynamicStateCount = (uint32_t)dynamics.size();

		// Load our SPIR-V shaders.
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{};

		// Vertex stage of the pipeline
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = LoadShader("Shaders/basic.vert.spirv");
		shader_stages[0].pName = "main";

		// Fragment stage of the pipeline
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = LoadShader("Shaders/basic.frag.spirv");
		shader_stages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipe{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipe.stageCount = (uint32_t)shader_stages.size();
		pipe.pStages = shader_stages.data();
		pipe.pVertexInputState = &vertex_input;
		pipe.pInputAssemblyState = &input_assembly;
		pipe.pRasterizationState = &raster;
		pipe.pColorBlendState = &blend;
		pipe.pMultisampleState = &multisample;
		pipe.pViewportState = &viewport;
		pipe.pDepthStencilState = &depth_stencil;
		pipe.pDynamicState = &dynamic;

		// We need to specify the pipeline layout and the render pass description up front as well.
		pipe.renderPass = renderPass;
		pipe.layout = m_PipelineLayout;

		VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipe, nullptr, &m_GraphicsPipline));


		// Pipeline is baked, we can delete the shader modules now.
		vkDestroyShaderModule(device, shader_stages[0].module, nullptr);
		vkDestroyShaderModule(device, shader_stages[1].module, nullptr);
	}

	void Renderer::InitBuffers()
	{

		VkDevice device = cubed::GetVulkanInfo()->Device;

		m_VertexBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		m_IndexBuffer.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		//std::string path = Walnut::OpenFileDialog::OpenFile(".obj");
		//object = obj::parseObj(path.c_str());

		
		glm::vec3 vertices[8] = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.0f),
			glm::vec3(-0.5f, 0.5f, 0.0f),
			glm::vec3(-0.5f, -0.5f, -1.0f),
			glm::vec3(0.5f, -0.5f, -1.0f),
			glm::vec3(0.5f, 0.5f, -1.0f),
			glm::vec3(-0.5f, 0.5f, -1.0f)
		};

		uint32_t indices[36] = {
			0, 1, 2, 2, 3, 0,
			1, 5, 6, 6, 2, 1,
			7, 6, 5, 5, 4, 7,
			4, 0, 3, 3, 7, 4,
			3, 2, 6, 6, 7, 3,
			4, 5, 1, 1, 0, 4
		};
		
		/*
		const int vertexCount = object.vertexPositions.size();
		const int indexCount = object.indicies.size();

		//get vertices and indicies from object
		glm::vec3 vertices[42] = {};
		for (int i = 0; i < object.vertexPositions.size(); i++)
		{
			vertices[i] = object.vertexPositions[i];
		}

		uint32_t indices[240] = {};
		for (int i = 0; i < object.indicies.size(); i++)
		{
			indices[i] = object.indicies[i];
		}
		
		std::cout << "Loaded " << object.vertexPositions.size() << " vertices" << std::endl;
		std::cout << "Loaded " << object.indicies.size() << " indicies" << std::endl;
		*/
		m_VertexBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		m_IndexBuffer.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		CreateOrResizeBuffer(m_VertexBuffer, sizeof(vertices));
		CreateOrResizeBuffer(m_IndexBuffer, sizeof(indices));

		glm::vec3* vbMemory;
		vkMapMemory(device, m_VertexBuffer.BufferMemory, 0, sizeof(vertices), 0, (void**)&vbMemory);
		memcpy(vbMemory, vertices, sizeof(vertices));

		uint32_t* ibMemory;
		vkMapMemory(device, m_IndexBuffer.BufferMemory, 0, sizeof(indices), 0, (void**)&ibMemory);
		memcpy(ibMemory, indices, sizeof(indices));

		VkMappedMemoryRange range[2] = {};
		range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory = m_VertexBuffer.BufferMemory;
		range[0].size = VK_WHOLE_SIZE;
		range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[1].memory = m_IndexBuffer.BufferMemory;
		range[1].size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(device, 2, range);
		vkUnmapMemory(device, m_VertexBuffer.BufferMemory);
		vkUnmapMemory(device, m_IndexBuffer.BufferMemory);


	};


	void Renderer::CreateOrResizeBuffer(Buffer& buffer, uint64_t new_size)
	{
		VkDevice device = cubed::GetVulkanInfo()->Device;

		if (buffer.Handle != VK_NULL_HANDLE)
			vkDestroyBuffer(device, buffer.Handle, nullptr);
		if (buffer.BufferMemory != VK_NULL_HANDLE)
			vkFreeMemory(device, buffer.BufferMemory, nullptr);

		VkBufferCreateInfo bufferCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = new_size;
		bufferCI.usage = buffer.usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK(vkCreateBuffer(device, &bufferCI, nullptr, &buffer.Handle));

		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(device, buffer.Handle, &req);
		//bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;
		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = req.size;
		alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
		VK_CHECK(vkAllocateMemory(device, &alloc_info, nullptr, &buffer.BufferMemory));
		

		VK_CHECK(vkBindBufferMemory(device, buffer.Handle, buffer.BufferMemory, 0));
		buffer.Size = req.size;

	}

	void Renderer::CreateOffscreenFramebuffer(ImGui_ImplVulkanH_Window* mainWindow) {
		std::cout << "Creating offscreen framebuffer" << std::endl;
		VkDevice device = cubed::GetVulkanInfo()->Device;

		// Create offscreen image
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = mainWindow->Width;
		imageInfo.extent.height = mainWindow->Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &m_OffscreenImage);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to create offscreen image: " << result << std::endl;
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_OffscreenImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &m_OffscreenImageMemory);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to allocate memory for offscreen image: " << result << std::endl;
			return;
		}

		result = vkBindImageMemory(device, m_OffscreenImage, m_OffscreenImageMemory, 0);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to bind memory for offscreen image: " << result << std::endl;
			return;
		}

		// Create image view
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_OffscreenImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &viewInfo, nullptr, &m_OffscreenImageView);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to create image view for offscreen image: " << result << std::endl;
			return;
		}

		// Create render pass
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_OffscreenRenderPass);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to create render pass for offscreen framebuffer: " << result << std::endl;
			return;
		}

		// Create framebuffer
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_OffscreenRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_OffscreenImageView;
		framebufferInfo.width = mainWindow->Width;
		framebufferInfo.height = mainWindow->Height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_OffscreenFramebuffer);
		if (result != VK_SUCCESS) {
			std::cerr << "Failed to create offscreen framebuffer: " << result << std::endl;
			return;
		}
		std::cout << "Offscreen framebuffer created" << std::endl;
	}


}