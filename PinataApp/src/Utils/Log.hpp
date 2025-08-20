#pragma once
#include <string>
#include <vector>
#include <imgui.h>


// Add a Array of strings
inline static std::vector<std::string> Logs = {};
inline static std::vector<std::string> Keys = {};

inline static bool ForceDown = false;
inline static int ForceDownCounter = 0; //Used to delay the force down by 1 frame to make sure the log is drawn before scrolling down



static enum EType {
	Normal,
	Warning,
	Error,
	GREEN,
	BLUE,
	PURPLE
};


//Logs a Int
inline static void Log(int i)
{
	Logs.push_back(std::to_string(i));
	Keys.push_back(" ");
	ForceDown = true;
}

//Logs a uint32_t
inline static void Log(uint32_t i)
{
	Logs.push_back(std::to_string(i));
	Keys.push_back(" ");
	ForceDown = true;
}

//Logs a Float
inline static void Log(float f)
{
	Logs.push_back(std::to_string(f));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Double
inline static void Log(double d)
{
	Logs.push_back(std::to_string(d));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Bool
inline static void Log(bool b)
{
	Logs.push_back(std::to_string(b));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Char
inline static void Log(char c)
{
	Logs.push_back(std::to_string(c));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a ImGui Vector2
inline static void Log(ImVec2 v)
{
	Logs.push_back("X: " + std::to_string(v.x) + " Y: " + std::to_string(v.y));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a ImGui Vector3
inline static void Log(ImVec4 v)
{
	Logs.push_back("X: " + std::to_string(v.x) + " Y: " + std::to_string(v.y) + " Z: " + std::to_string(v.z));
	Keys.push_back(" ");
	ForceDown = true;
}

inline static void Log(std::vector<unsigned char> Bytes)
{
	std::string str = "";
	for (int i = 0; i < Bytes.size(); i++)
	{
		//use mod to add a space every 4 bytes
		if (i % 4 == 0 && i != 0) {
			str += " | ";
		}
		//format the byte to hex numbers and letters
		str += " " + std::to_string(Bytes[i] >> 4) + std::to_string(Bytes[i] & 0xF);
	}
	Logs.push_back(str);
	Keys.push_back("PURPLE");
}

//Clears the log
inline static void ClearLog()
{
	Logs.clear();
	Keys.clear();
	ForceDown = true;
}

//Draws the log using ImGui
inline static void DrawLog()
{
	ImGui::BeginChild("Log");
	
	for (int i = 0; i < Logs.size(); i++)
	{
		if (Keys[i] == "Err"){
			float pulse = (sin((ImGui::GetTime() + i) * 5) + 1.0f) * 0.5f; // Calculate pulse value
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, static_cast<int>(pulse * 127), static_cast<int>(pulse * 127), 255));
		}else if (Keys[i] == "Warn"){
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
		}else if (Keys[i] == "GREEN") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		}else if (Keys[i] == "BLUE") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 255, 255));
		}else if (Keys[i] == "PURPLE") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 127, 255, 255));
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
		}
		//set text padding on the left
		ImGui::SetCursorPosX(10);
		ImGui::Text(Logs[i].c_str());
		ImGui::PopStyleColor();
		//force scroll down
		if (ForceDown) {
			if (ForceDownCounter == 1) {
				ImGui::SetScrollY(ImGui::GetScrollMaxY() + 1000);
				ForceDown = false;
				ForceDownCounter = 0;
			}
			else {
				ImGui::SetScrollY(ImGui::GetScrollMaxY() + 1000);
				ForceDownCounter++;
			}
		}

	}
	ImGui::EndChild();
	
}

//Removes the last log
inline static void RemoveLastLog()
{
	Logs.pop_back();
	Keys.pop_back();
	ForceDown = true;
}

//Removes a specific log
inline static void RemoveFromLog(int i)
{
	Logs.erase(Logs.begin() + i);
	Keys.erase(Keys.begin() + i);
	ForceDown = true;
}

//Uses a Key to only make sure item is logged once (Useful for logging values that change every frame)
inline static void KeyLogString(std::string Key, std::string Value)
{
	bool Found = false;

	for (int i = 0; i < Keys.size(); i++)
	{
		if (Keys[i] == Key)
		{
			RemoveFromLog(i);
			Logs.push_back(Value);
			Keys.push_back(Key);
			Found = true;
			ForceDown = true;
			break;
		}
		
	}
	if (!Found) {

		Logs.push_back(Value);
		Keys.push_back(Key);
		ForceDown = true;

	}
}



//Logs a String
inline static void Log(std::string str, EType Type)
{
	switch (Type)
	{
	case Normal:
		Logs.push_back(str);
		Keys.push_back(" ");
		break;
	case Warning:
		Logs.push_back(str);
		Keys.push_back("Warn");
		break;
	case Error:
		Logs.push_back(str);
		Keys.push_back("Err");
		break;
	case GREEN:
		Logs.push_back(str);
		Keys.push_back("GREEN");
		break;
	case BLUE:
		Logs.push_back(str);
		Keys.push_back("BLUE");
		break;
	case PURPLE:
		Logs.push_back(str);
		Keys.push_back("PURPLE");
		break;
	}
	ForceDown = true;
}



