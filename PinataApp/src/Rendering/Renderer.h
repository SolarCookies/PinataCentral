#pragma once
#include <vulkan/vulkan.h>
#include "../Utils/VulkanUtils.h"
#include <filesystem>
#include <fstream>
#include "glm/glm.hpp"

//#include "../Utils/OpenOBJ.h"

namespace cubed {

	struct Buffer {
		VkBuffer Handle = nullptr;
		VkDeviceMemory BufferMemory = nullptr;
		VkDeviceSize Size = 0;
		VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	};

	class Renderer {
	public:

		VkImage m_OffscreenImage = VK_NULL_HANDLE;
		VkDeviceMemory m_OffscreenImageMemory = VK_NULL_HANDLE;
		VkImageView m_OffscreenImageView = VK_NULL_HANDLE;
		VkFramebuffer m_OffscreenFramebuffer = VK_NULL_HANDLE;
		VkRenderPass m_OffscreenRenderPass = VK_NULL_HANDLE;

		void Init(ImGui_ImplVulkanH_Window* mainWindow);
		void Shutdown();

		void Render(ImGui_ImplVulkanH_Window* mainWindow, int Width, int Height);
		void RenderUI();

		VkImageView GetOffscreenImageView() const { return m_OffscreenImageView; }

		

	private:
		VkShaderModule LoadShader(const std::filesystem::path& path);
		void InitPipeline(ImGui_ImplVulkanH_Window* mainWindow);
		void InitBuffers();
		void CreateOrResizeBuffer(Buffer& buffer, uint64_t new_size);
		void CreateOffscreenFramebuffer(ImGui_ImplVulkanH_Window* mainWindow);

	private:
		VkPipeline m_GraphicsPipline = nullptr;
		VkPipelineLayout m_PipelineLayout = nullptr;
		
		Buffer m_VertexBuffer, m_IndexBuffer;


		struct PushConstants {
			glm::mat4 ViewProjection;
			glm::mat4 Transform;
		};
		
		PushConstants m_PushConstants;

		//obj::MeshData object;

		glm::vec3 m_CubePosition = glm::vec3(0);
		glm::vec3 m_CubeRotation = glm::vec3(0);

	};
	
}