#pragma once
#include <imgui.h>

class GUI;

class Page {

	public:
	virtual ~Page() = default;
	virtual void render(GUI &gui) = 0;

};