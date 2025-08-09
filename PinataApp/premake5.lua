project "PinataApp"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "../vendor/imgui",
      "../vendor/GLFW/include",
      "../vendor/glm",
      "../vendor/zlib_x64-windows-static/include",
      "../vendor/spdlog/include",
      "../vendor/yaml-cpp/include",
      "../Walnut/Platform/GUI",
      "../Walnut/Platform/Headless",
      "../venderhalf/include",

      "../Walnut/Source",

      "%{IncludeDir.VulkanSDK}",
   }

   links
   {
       "Walnut"
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS", "NOMINMAX"}
      buildoptions { "/openmp" } -- Enable OpenMP for MSVC
      buildoptions { "-DVCPKG_DISABLE" } -- Disable vcpkg integration

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"
      links { "../vendor/zlib_x64-windows-static/lib/zlibd.lib" } -- Link zlibd.lib for Debug

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"
      links { "../vendor/zlib_x64-windows-static/lib/zlib.lib" } -- Link zlib.lib for Release

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"
      links { "../vendor/zlib_x64-windows-static/lib/zlib.lib" } -- Link zlib.lib for Dist