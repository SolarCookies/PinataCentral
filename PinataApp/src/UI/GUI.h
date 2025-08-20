#pragma once
#include <vector>
#include <memory>
#include "Page.h"

class GUI {
	public:
		std::unique_ptr<Page> CurrentPage;
		bool HasInitialized = false;

		void init();
		void render();

};
