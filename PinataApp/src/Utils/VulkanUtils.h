#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>
#include "imgui.h"
#include <iostream>

#include "backends/imgui_impl_vulkan.h"

namespace vkb {

	inline const std::string kto_string(VkResult result)
	{
		switch (result)
		{
#define STR(r)   \
	case VK_##r: \
		return #r
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_FRAGMENTED_POOL);
			STR(ERROR_UNKNOWN);
			STR(ERROR_OUT_OF_POOL_MEMORY);
			STR(ERROR_INVALID_EXTERNAL_HANDLE);
			STR(ERROR_FRAGMENTATION);
			STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			STR(PIPELINE_COMPILE_REQUIRED);
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			STR(ERROR_VALIDATION_FAILED_EXT);
			STR(ERROR_INVALID_SHADER_NV);
#undef STR
		default:
			return "UNKNOWN_ERROR";
		}
	}

//#define VK_CHECK(x){ VkResult err = x; if (err) { throw std::runtime_error("Vulkan error: " + std::to_string(err)); } }
	//VK_CHECKCOUT
#define VK_CHECK(x){ VkResult err = x; if (err) { std::cout << "Vulkan error: " << vkb::kto_string(err) << std::endl; } }

}

namespace cubed {

	static ImGui_ImplVulkan_InitInfo* GetVulkanInfo() {
		return ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_InitInfo*)ImGui::GetIO().BackendRendererUserData : NULL;
	}


}