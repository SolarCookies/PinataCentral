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
#include "UI/FileBrowser/FileBrowser.h"
#include "Rendering/Renderer.h"
#include "Utils/Log.hpp"

//working directory
std::string workingDirectory = std::filesystem::current_path().string();

//Settings.ini location
std::string settingsPath = workingDirectory + "/settings.ini";

//Settings window toggle
bool m_ShowSettingsWindow = false;

//Path to .pkg bundles
char path[256] = "C:/";



class ExampleLayer : public Walnut::Layer
{
public:

	//This runs every frame and is used to render the UI
	virtual void OnUIRender() override
	{

		
		//get popup data from settings.ini
		mINI::INIFile file(settingsPath);
		mINI::INIStructure ini;
		file.read(ini);
		bool ShowPopup = ini["Settings"]["ShowPopup"] == "true";

		if (ShowPopup)
		{
			std::string PopupMessage = ini["Settings"]["PopupMessage"];
			bool HasProgressBar = ini["Settings"]["HasProgressBar"] == "true";

			if (HasProgressBar)
			{
				float Progress = std::stof(ini["Settings"]["Progress"]) / 100.0f;
				//fullscreen next window
				ImGui::SetNextWindowPos(ImVec2(100, 100));
				ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 200));
				ImGui::SetNextWindowBgAlpha(0.5f);

				ImGui::Begin("Progress");
				//center text
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(PopupMessage.c_str()).x) / 2);
				ImGui::Text(PopupMessage.c_str());
				ImGui::ProgressBar(Progress);
				
				ImGui::End();
			}
			else
			{
				ImGui::Begin("Message");
				ImGui::Text(PopupMessage.c_str());
				ImGui::End();
			}


			//m_Renderer.RenderUI();
			return;
		}
		
		timer.Reset();

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

		m_FileBrowser.RenderFileBrowser(RenderMode::List);

		if (ImGui::Begin("Details"))
		{
			ImGui::Text("Details");
			ImGui::Text("FPS: %.2f", 1000.0f / m_lastRenderTime);
			
		}
		ImGui::End();

		//ImGui::SetNextWindowBgAlpha(0.0f);
		

		//Walnut::OpenFileDialog::DemoFileDialogWindow(); //Uncomment to show demo file dialog window to test functionality of OpenFileDialog.h
		
		if (timerUIUpdate.ElapsedMillis() > 100.0f)
		{
			m_lastRenderTime = timer.ElapsedMillis();
			timerUIUpdate.Reset();
		}

		ImGui::Begin("Log");
		if (ImGui::Button("Clear Log"))
		{
			ClearLog();
		}
		ImGui::SameLine();
		if (ImGui::Button("Copy Log"))
		{
			//copy log to clipboard
			std::string log = "";
			for (int i = 0; i < Logs.size(); i++)
			{
				if (Keys[i] == " ")
					log += Logs[i] + "\n";
				else
				log += Logs[i] + " Type: " + Keys[i] + "\n";
			}
			ImGui::SetClipboardText(log.c_str());
		}
		DrawLog();
		ImGui::End();


		//m_Renderer.RenderUI();
		
		/*
		// Create an ImGui window for the viewport
		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_Viewport.m_ViewportWidth = viewportSize.x;
		m_Viewport.m_ViewportHeight = viewportSize.y;

		// Get the offscreen image view
		//VkImageView offscreenImageView = m_Renderer.GetOffscreenImageView();

		// Display the offscreen image in the ImGui window
		//ImGui::Image((void*)(intptr_t)m_Renderer.GetOffscreenImageView(), viewportSize);
		ImGui::End();
		*/

	}

	//This runs every frame and is used to render the 3d scene
	virtual void OnRender() override
	{
		
		
		// Render the model to the offscreen framebuffer
		//m_Renderer.Render(Walnut::Application::GetMainWindowData(), m_Viewport.m_ViewportWidth, m_Viewport.m_ViewportHeight);
		//std::cout << "Rendered" << std::endl;
		
	}

	//This runs on begin play and is used to initialize the renderer along with other things that need to be initialized before the app renders for the first time
	virtual void OnAttach() override
	{

		
		//m_Renderer.Init(Walnut::Application::GetMainWindowData());
		//m_Viewport.run();
	}

private:
	//Timer for fps counter
	Walnut::Timer timer;

	//Timer for UI updates (How often the fps counter updates)
	Walnut::Timer timerUIUpdate;

	float m_lastRenderTime = 0.0f;

	//ToDo: Render Vulkan Pipeline to the viewport rather then the main window
	Viewport m_Viewport;

	//File Browser handles the displaying and user interaction with the .pkg files
	FileBrowser m_FileBrowser;

	//Renderer handles the rendering of models (I followed a tutorial to get this working thats why the namespace is "cubed")
	//cubed::Renderer m_Renderer;


};

//This is the entry point for the application
Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Pinata Central";
	spec.CustomTitlebar = true;
	spec.UseDockspace = true;
	

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
			if (ImGui::MenuItem("Open .obj"))
			{
				std::string path = Walnut::OpenFileDialog::OpenFile(".obj");
				if (!path.empty())
				{
					

				}
				
				
			}
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}

			ImGui::EndMenu();
		}
		
	});
	
	return app;
}