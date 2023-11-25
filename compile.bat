@echo off
set VULKAN_SDK=C:\VulkanSDK\1.3.268.0
set GLSLC=%VULKAN_SDK%\Bin\glslc.exe

set SHADERS_DIR=shaders

for %%f in (%SHADERS_DIR%\*.vert %SHADERS_DIR%\*.frag) do (
    %GLSLC% "%%f" -o "%%f.spv"
)

echo compile shader success