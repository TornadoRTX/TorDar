@set script_dir=%~dp0

@set build_dir=%script_dir%\..\build-debug-msvc2022
@set build_type=Debug
@set conan_profile=scwx-win64_msvc2022
@set generator=Visual Studio 17 2022
@set qt_base=C:/Qt
@set qt_arch=msvc2022_64

:: Assign user-specified build directory
@if not "%~1"=="" set build_dir=%~1

:: Perform common setup
@call lib\setup-common.bat

@pause
