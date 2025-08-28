#include "EditorSelectionPage.h"
#include "../../GlobalSettings.h"
#include "PackageManager/PackageManager.h"
#include "../GUI.h"
#include "SaveManager/SaveManager.h"
#include "BundleManager/BundleManager.h"
#include"ExtractAllPage.h"

void EditorPage::render(GUI& gui)
{
	ImGui::Begin("Editor Selection");

	{
		//Set child background color to white
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 0.6f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); // Set border color to black
		ImGui::BeginChild("PackageManagerButton", ImVec2(300, 120), true);
		//make button blue

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
		//Add Decription Text
		ImGui::TextWrapped("Used to browse and modify");

		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(".pkg files").x) / 2); // Center the text horizontally
		ImGui::Text(".pkg files");
		ImGui::PopStyleColor();

		//Center the button on the bottom of the child window
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		if (ImGui::Button("Package", ImVec2(200, 50))) {
			gui.CurrentPage = std::make_unique<PackageManager>();
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//ImGui::SameLine();

	{
		//Set position of the second child window to the right of the first one
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 320); // Move to the right of the first child
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 125); // Keep the Y position the same


		//Set child background color to white
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

		//Deactivated colors
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

		ImGui::BeginChild("DebugManagerButton", ImVec2(300, 120), true);
		//make button blue

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
		//Add Decription Text
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Used to browse and modify the").x) / 2); // Center the text horizontally
		ImGui::Text("Used to browse and modify the");
		//set the text to center
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("debug_pack.bin").x) / 2); // Center the text horizontally
		ImGui::Text("debug_pack.bin");
		ImGui::PopStyleColor();

		//Center the button on the bottom of the child window
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		if (ImGui::Button("Coming Soon", ImVec2(200, 50))) {
			// Handle button click
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	{
		//Set position of the second child window to the right of the first one
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 320 * 2); // Move to the right of the first child
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 125); // Keep the Y position the same


		//Set child background color to white
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 1.0f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));

		ImGui::BeginChild("BundleManagerButton", ImVec2(300, 120), true);
		//make button blue

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
		//Add Decription Text
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Used to browse and modify the").x) / 2); // Center the text horizontally
		ImGui::Text("Used to browse and modify the");
		//set the text to center
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Localization Bundles").x) / 2); // Center the text horizontally
		ImGui::Text("Localization Bundles");
		ImGui::PopStyleColor();

		//Center the button on the bottom of the child window
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		if (ImGui::Button("Bundle", ImVec2(200, 50))) {
			// Handle button click
			gui.CurrentPage = std::make_unique<BundleManager>();
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	if(!IsTIP())
	{
		//Set position of the second child window to the right of the first one
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 320 * 3); // Move to the right of the first child
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 125); // Keep the Y position the same


		//Set child background color to white
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 0.3f, 0.7f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 1.0f, 1.0f)); // Set border color to black

		//Deactivated colors
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

		ImGui::BeginChild("ShaderManagerButton", ImVec2(300, 120), true);
		//make button blue

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
		//Add Decription Text
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Used to browse and modify the").x) / 2); // Center the text horizontally
		ImGui::Text("Used to browse and modify the");
		//set the text to center
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("shader data.wad").x) / 2); // Center the text horizontally
		ImGui::Text("Shader .wad");
		ImGui::PopStyleColor();

		//Center the button on the bottom of the child window
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.1f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		if (ImGui::Button("Coming Soon", ImVec2(200, 50))) {
			// Handle button click
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	if (!IsTIP())
	{
		//Set position of the second child window to the right of the first one
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 320 * 4); // Move to the right of the first child
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 125); // Keep the Y position the same


		//Set child background color to white
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
		//ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set border color to black

		//Deactivated colors
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

		ImGui::BeginChild("SaveManagerButton", ImVec2(300, 120), true);
		//make button blue

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
		//Add Decription Text
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Used to browse and modify the").x) / 2); // Center the text horizontally
		ImGui::Text("Used to browse and modify the");
		//set the text to center
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Save Files").x) / 2); // Center the text horizontally
		ImGui::Text("Save Files");
		ImGui::PopStyleColor();

		//Center the button on the bottom of the child window
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		if (ImGui::Button("Coming Soon", ImVec2(200, 50))) {
			// Handle button click
			gui.CurrentPage = std::make_unique<SaveManager>();
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();




		
	}

	//Set position of the second child window to the right of the first one
	ImGui::SetCursorPosX(ImGui::GetCursorPosX()); // Move to the right of the first child
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 125 + 125); // Keep the Y position the same


	//Set child background color to white
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set border color to black

	//Deactivated colors
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	ImGui::BeginChild("ExportManagerButton", ImVec2(300, 120), true);
	//make button blue

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set text color to black
	//Add Decription Text
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Used to mass export the").x) / 2); // Center the text horizontally
	ImGui::Text("Used to mass export the");
	//set the text to center
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(".pkg Files").x) / 2); // Center the text horizontally
	ImGui::Text(".pkg Files");
	ImGui::PopStyleColor();

	//Center the button on the bottom of the child window
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60); // Adjust the Y position to place the button at the bottom
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) / 2); // Center the button horizontally
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	if (ImGui::Button("Extract", ImVec2(200, 50))) {
		// Handle button click
		gui.CurrentPage = std::make_unique<ExtractPage>();
	}
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::End();

}
