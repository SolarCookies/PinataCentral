@echo off

pushd ..
vendor\bin\premake\Windows\premake5.exe --file=Build-Walnut-PinataProject.lua vs2022
popd
pause