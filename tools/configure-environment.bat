@setlocal enabledelayedexpansion

@set script_dir=%~dp0
@set venv_path=%script_dir%\..\.venv

:: Assign user-specified Python Virtual Environment
@if not "%~1"=="" (
    if /i "%~1"=="none" (
        set venv_path=
    ) else (
        set venv_path=%~f1
    )
)

:: Activate Python Virtual Environment
@if defined venv_path (
    echo Activating Python Virtual Environment: %venv_path%
    python -m venv %venv_path%
    call %venv_path%\Scripts\activate.bat
)

:: Install Python packages
python -m pip install --upgrade pip
pip install --upgrade -r "%script_dir%\..\requirements.txt"

:: Configure default Conan profile
@conan profile detect -e

:: Conan profiles
@set profile_count=1
@set /a last_profile=profile_count - 1
@set conan_profile[0]=scwx-windows_msvc2022_x64

:: Install Conan profiles
@for /L %%i in (0,1,!last_profile!) do @(
    set "profile_name=!conan_profile[%%i]!"
    conan config install "%script_dir%\conan\profiles\!profile_name!" -tf profiles
)

:: Deactivate Python Virtual Environment
@if defined venv_path (
    call %venv_path%\Scripts\deactivate.bat
)

@pause
