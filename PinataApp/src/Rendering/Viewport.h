#pragma once

#include "imgui.h"
#include "Walnut/Application.h"
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class Viewport
{
public:
	Viewport() {
	};
	~Viewport() {
	};

    float m_ViewportWidth = 100.0f;
    float m_ViewportHeight = 100.0f;

};