#pragma once

#include <string>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <commdlg.h>

#include "Walnut/Application.h"

#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <filesystem>
#include "ZLibHelpers.h"
#include <fstream>

#include "imgui.h"


#pragma comment(lib, "shell32.lib")

namespace Walnut
{
	class OpenFileDialog
	{
	public:
		//Returns Empty string if no file is selected
		inline static  std::string OpenFile(const char* filter) {

			OPENFILENAMEA ofn;
			char szFile[260] = { 0 };

			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindowHandle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			if (GetOpenFileNameA(&ofn) == TRUE) {
				return ofn.lpstrFile;
			}
			return "";

		};

		inline static bool AskSaveFile(BYTES& Data, std::string Filename) {
			//get file extension from end of filename
			std::string extension = Filename.substr(Filename.find_last_of(".") + 1);
			//convert to char*
			char* ext = &extension[0];

			//filename is formated like this aid_1_2_3_4,102301923,2.03
			//we want to remove the , and everything after it then append the extension
			Filename = Filename.substr(0, Filename.find_last_of(","));
			Filename = Filename.substr(0, Filename.find_last_of(","));
			//append extension
			Filename += "." + extension;

			OPENFILENAMEA ofn;
			char szFile[260] = { 0 };
			strcpy(szFile, Filename.c_str());
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindowHandle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = ext;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
			if (GetSaveFileNameA(&ofn) == TRUE) {
				std::cout << "Saving file: " << ofn.lpstrFile << std::endl;
				std::string path = ofn.lpstrFile;
				//split path into path and filename
				std::string Path = path.substr(0, path.find_last_of("\\") + 1);
				std::string Filename = path.substr(path.find_last_of("\\") + 1);
				WriteFile(Path, Filename, Data);
				return true;
			}
			return false;
		}

		//Writes a file to a specified path, if the path does not exist it will create it
		inline static void WriteFile(std::string& Path, std::string& Filename, BYTES& Data) {
			if (!std::filesystem::create_directories(Path) && !std::filesystem::exists(Path)) {
				//  Log("Cannot create path \\" + Path + "\\", Error);
				std::cout << "Cannot create path \\" << Path << "\\";
			}
			std::ofstream File(Path + Filename, std::ios::binary);
			if (File.is_open()) {
				File.write(reinterpret_cast<const char*>(Data.data()), Data.size());
				File.close();
				// Log("Successfully wrote file " + Path + Filename, GREEN);
			}
			else {
				// Log("Failed to open file " + Path + Filename, Error);
			}

		}

		//Opens Folder Dialog and returns the path
		inline static std::string OpenFolder() {
            BROWSEINFO bi = { 0 };
            wchar_t szTitle[] = L"Browse for folder...";
            bi.lpszTitle = szTitle;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl != 0) {
                // get the name of the folder
                wchar_t path[MAX_PATH];
                SHGetPathFromIDListW(pidl, path);
                // free memory used
                IMalloc* imalloc = 0;
                if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                    imalloc->Free(pidl);
                    imalloc->Release();
                }
                // convert wchar_t* to std::string
                char charPath[MAX_PATH];
                wcstombs(charPath, path, MAX_PATH);
                return charPath;
            }
            return "";
        }

		//Display demo file dialog window for testing functionality
		inline static  void DemoFileDialogWindow() {
			//File Browser Window
			if (ImGui::Begin("Open File Dialog Demonstration"))
			{
				if (ImGui::Button("Open File"))
				{
					std::string path = Walnut::OpenFileDialog::OpenFile("All Files\0*.*\0");
					if (!path.empty())
					{
						
					}
				}

				if (ImGui::Button("Open Folder"))
				{
					std::string path = Walnut::OpenFileDialog::OpenFolder();
					if (!path.empty())
					{
						
					}
				}

				ImGui::End();
			}
		}

		//Returns true if file exists
		inline static  bool FileExists(const std::string& path) {
			return std::filesystem::exists(path);

		}

		//Returns the number of files in a directory
		inline static  uint32_t GetFileCount(const std::string& path) {
			return std::filesystem::directory_entry(path).is_directory();
		}

		//Funtion that returns all files in a directory
		inline static  std::vector<std::string> GetFilesInDirectory(const std::string& path) {
			std::vector<std::string> files;
			//files in path
			auto directory_iterator = std::filesystem::directory_iterator(path);
			//The only files we care about are .pkg files and they count up ie . 1.pkg, 2.pkg, 3.pkg
			int count = 0;
			for (const auto& entry : directory_iterator)
			{
				if (entry.path().extension() == ".pkg")
				{
					count++;
					//Push path count.pkg that way they are automatically sorted
					if (FileExists(path + "\\" + std::to_string(count) + ".pkg")){
						files.push_back(path + "\\" + std::to_string(count) + ".pkg");
					}
					

				}
			}
			return files;
		}

		//Sets the current IfStream position to 'Offset' from the beginning of the file
		inline static void SeekBeg(std::ifstream& Stream, uint32_t Offset) {
			Stream.seekg(Offset, std::ios::beg);
		}

		//Sets the current IfStream position to 'Offset' from the current position
		inline static void SeekCur(std::ifstream& Stream, uint32_t Offset) {
			Stream.seekg(Offset, std::ios::cur);
		}

		//Sets the current IfStream position to 'Offset' from the end of the file
		inline static void SeekEnd(std::ifstream& Stream, uint32_t Offset) {
			Stream.seekg(Offset, std::ios::end);
		}

		//Reads 'Size' Bytes at 'Offset' then restores the seek position
		inline static BYTES Read_Bytes(std::ifstream& Stream, uint32_t Offset, uint32_t Size) {
			//Save current position
			uint32_t CurrentPosition = Stream.tellg();
			//Seek to offset
			SeekBeg(Stream, Offset);
			//Read Bytes
			BYTES Data(Size);
			Stream.read(reinterpret_cast<char*>(Data.data()), Size);
			//Restore position
			SeekBeg(Stream, CurrentPosition);
			return Data;
			
		}

		//Reads bytes from current file position
		inline static BYTES Read_Bytes(std::ifstream& Stream, uint32_t Size) {
			BYTES Data(Size);
			Stream.read(reinterpret_cast<char*>(Data.data()), Size);
			//forword offset by size
			//SeekCur(Stream, Size);
			return Data;
		}

		inline static BYTES CopyBytes(BYTES& Data, uint32_t StartOffset, uint32_t Length) {
			BYTES Result;
			for (int i = StartOffset; i < StartOffset + Length; i++) {
				if (Data.size() > i) {
					Result.push_back(Data[i]);
				}
			}
			return Result;
		}

		inline static void SetIntAtOffset(BYTES& Data, uint32_t Offset, uint32_t Value, bool BigEndian) {
			BYTES IntBytes = Zlib::ConvertIntToBytes(Value, BigEndian);
			for (int i = 0; i < 4; i++) {
				Data[Offset + i] = IntBytes[i];
				//std::cout << "Byte: " << i << " Value: " << (int)IntBytes[i] << std::endl;
			}
		}
	};
}