@setlocal enabledelayedexpansion

@set script_dir=%~dp0

:: Install Python packages
@pip install --upgrade -r "%script_dir%\..\requirements.txt"

:: Configure default Conan profile
@conan profile detect -e

:: Conan profiles
@set profile_count=1
@set /a last_profile=profile_count - 1
@set conan_profile[0]=scwx-win64_msvc2022

:: Install Conan profiles
@for /L %%i in (0,1,!last_profile!) do @(
    set "profile_name=!conan_profile[%%i]!"
    conan config install "%script_dir%\conan\profiles\%profile_name%" -tf profiles
)

@pause
