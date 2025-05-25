@echo off
set script_dir=%~dp0

set cmake_args=-B "%build_dir%" -S "%script_dir%\..\.." ^
    -G "%generator%" ^
    -DCMAKE_PREFIX_PATH="%qt_base%/%qt_version%/%qt_arch%" ^
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="%script_dir%\..\..\external\cmake-conan\conan_provider.cmake" ^
    -DCONAN_HOST_PROFILE=%conan_profile% ^
    -DCONAN_BUILD_PROFILE=%conan_profile%

if defined build_type (
    set cmake_args=%cmake_args% ^
        -DCMAKE_BUILD_TYPE=%build_type% ^
        -DCMAKE_CONFIGURATION_TYPES=%build_type%
)
@echo on

mkdir "%build_dir%"
cmake %cmake_args%
