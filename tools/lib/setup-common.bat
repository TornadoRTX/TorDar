@set script_dir=%~dp0

:: Import common paths
@call lib\common-paths.bat

:: Install Python packages
pip install --upgrade -r "%script_dir%\..\..\requirements.txt"

@if defined build_type (
    :: Install Conan profile and packages
    call lib\setup-conan.bat
) else (
    :: Install Conan profile and debug packages
    set build_type=Debug
    call lib\setup-conan.bat

    :: Install Conan profile and release packages
    set build_type=Release
    call lib\setup-conan.bat

    :: Unset build_type
    set build_type=
)

:: Run CMake Configure
@call lib\run-cmake-configure.bat
