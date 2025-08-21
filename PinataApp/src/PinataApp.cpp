#include "Walnut/Application.h"

#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Utils/OpenFileDialog.h"
#include "Utils/ini.h"

#include <filesystem>
#include <iostream>

#include "GlobalSettings.h"

#include "Rendering/Viewport.h"
#include "UI/Pages/PackageManager/FileBrowser/FileBrowser.h"
#include "Rendering/Renderer.h"
#include "Utils/Log.hpp"
#include "Debug_Pack.h"
#include "Data_Wad.h"
#include "Bundle.h"


#include "UI/GUI.h"

//working directory
std::string workingDirectory = std::filesystem::current_path().string();

//Settings.ini location
std::string settingsPath = workingDirectory + "/Assets/settings.ini";

//Settings window toggle
bool m_ShowSettingsWindow = false;

//Path to .pkg bundles
char path[256] = "C:/";

class ExampleLayer : public Walnut::Layer
{
public:

	GUI m_GUI;

	//This runs every frame and is used to render the UI
	virtual void OnUIRender() override
	{
		//get popup data from settings.ini
		mINI::INIFile file(settingsPath);
		mINI::INIStructure ini;
		file.read(ini);
		bool ShowPopup = ini["Settings"]["ShowPopup"] == "true";


		//Show settings window if enabled
		if (m_ShowSettingsWindow)
		{
			ImGui::Begin("Settings", &m_ShowSettingsWindow);
			//Path to .pkg bundles
			ImGui::InputText(".pkg Path", path, 256);
			//button to select path
			if (ImGui::Button("Select Path"))
			{
				std::string spath = Walnut::OpenFileDialog::OpenFolder();
				if (!spath.empty())
				{
					strcpy_s(path, spath.c_str());
					mINI::INIFile file(settingsPath);
					mINI::INIStructure ini;
					ini["Settings"]["Path"] = path;
					file.write(ini);
				}
			}

			ImGui::End();
		}

		//ImGui::Begin("Pinata Central", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
		//m_GUI.render();
		//ImGui::End();
		
		if (!m_GUI.HasInitialized) {
			m_GUI.init();
		}
		
		m_GUI.render();
		

	}

	//This runs every frame and is used to render the 3d scene
	virtual void OnRender() override
	{
		
	}

	//This runs on begin play and is used to initialize the renderer along with other things that need to be initialized before the app renders for the first time
	virtual void OnAttach() override
	{
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	}

private:
	

};

//This is the entry point for the application
Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Pinata Central";
	spec.CustomTitlebar = true;
	spec.UseDockspace = true;
	spec.IconPath = "Assets/PinataCentralIcon.png"; //Path to the icon for the application
	spec.CenterWindow = true; //Center the window on the screen

	//load settings from settings.ini
	if (Walnut::OpenFileDialog::FileExists(settingsPath))
	{
		mINI::INIFile file(settingsPath);
		mINI::INIStructure ini;
		file.read(ini);
		strcpy_s(path, ini["Settings"]["Path"].c_str());
	}
	else {
		mINI::INIFile file(settingsPath);
		mINI::INIStructure ini;
		ini["Settings"]["Path"] = path;
		file.generate(ini);
	}

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();

	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Settings"))
				{
					m_ShowSettingsWindow = !m_ShowSettingsWindow;
				}
				if (std::filesystem::exists(std::filesystem::path(path).parent_path().string() + "/Startup.exe"))
				{
					if (ImGui::MenuItem("Open Debug Pack (WIP)"))
					{
						std::string path1 = Walnut::OpenFileDialog::OpenFile("Debug Pack\0*.bin\0\0");
						if (!path1.empty())
						{
							//Open the debug pack
							debug_pack::read(path1);
						}

					}
					if (ImGui::MenuItem("Open Data.wad (WIP)"))
					{
						std::string path1 = Walnut::OpenFileDialog::OpenFile("Data Wad\0*.wad\0\0");
						if (!path1.empty())
						{
							//Open the data wad
							Data_Wad::read(path1);
						}
					}
					
				}
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}

				ImGui::EndMenu();
			}

			if (std::filesystem::exists(std::filesystem::path(path).parent_path().string() + "/Startup.exe"))
			{
				if (ImGui::Button("Play Game"))
				{
					//Use pkg location and get one folder up and run Startup.exe
					std::string path1 = std::filesystem::path(path).parent_path().string() + "\\Startup.exe";
					Log("Running game at: " + path1, EType::Warning);
					std::string quotedPath = "\"" + path1 + "\"";
					system(quotedPath.c_str());
				}
			}
		});

	return app;
}

