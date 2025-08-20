#include "WelcomePage.h"
#include "../GUI.h"
#include "../../GlobalSettings.h"
#include "EditorSelectionPage.h"

void WelcomePage::render(GUI &gui)
{
	ImGui::Begin("Welcome");
	//center the text
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Welcome to Pinata Central, The Package Manager for Viva Pinata!").x) * 0.5f);
	ImGui::Text("Welcome to Pinata Central, The Package Manager for Viva Pinata!");
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Please select the path to your Viva Pinata exe/xex").x) * 0.5f);
	ImGui::Text("Please select the path to your Viva Pinata exe/xex");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Select Path").x) * 0.5f);
	//Add button to select the path
	if (ImGui::Button("Select Path")) {
		Path = Walnut::OpenFileDialog::OpenFolder();
	}
	// Display the selected path
	if (Path.empty()) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("No path selected.").c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No path selected.");
		goto End;
	}
	//Check to see if the path points to the correct location by checking if there is a file called "Viva Pinata.exe" in the path
	else if (std::filesystem::exists(Path + "/Viva Pinata.exe") && std::filesystem::exists(Path + "/bundles_packages/1.pkg")) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("Valid Path: " + Path).c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Valid Path: %s", Path.c_str());
	}
	else if (std::filesystem::exists(Path + "/default.xex") && std::filesystem::exists(Path + "/Beta/packages/1.pkg")) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("Valid Path: " + Path).c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Valid Path: %s", Path.c_str());
	}
	else {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("Invalid Path: " + Path).c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Path: %s", Path.c_str());
		goto End;
	}

	//If the path is valid, the rest will run
	// Add a button to continue and make it green
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Continue").x) * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
	if (ImGui::Button("Continue")) {
		UpdateRootDicrectory(Path);
		SaveSettings();
		gui.CurrentPage = std::make_unique<EditorPage>();
		
	}
	ImGui::PopStyleColor();


End:
	ImGui::End();
};