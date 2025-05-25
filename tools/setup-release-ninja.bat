@set script_dir=%~dp0

@set build_dir=%script_dir%\..\build-release-ninja
@set build_type=Release
@set conan_profile=scwx-win64_msvc2022
@set generator=Ninja
@set qt_base=C:/Qt
@set qt_arch=msvc2022_64

:: Assign user-specified build directory
@if not "%~1"=="" set build_dir=%~1

:: Perform common setup
@call lib\setup-common.bat

@pause
