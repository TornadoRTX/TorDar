@set script_dir=%~dp0

@set build_dir=%script_dir%\..\build-msvc2022
@set conan_profile=scwx-windows_msvc2022_x64
@set generator=Visual Studio 17 2022
@set qt_base=C:/Qt
@set qt_arch=msvc2022_64

:: Assign user-specified build directory
@if not "%~1"=="" set build_dir=%~1

:: Perform common setup
@call %script_dir%\lib\setup-common.bat

@pause
