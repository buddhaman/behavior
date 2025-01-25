@echo off
setlocal

rem Set paths for dependencies
set SDL2_PATH=external\sdl2
set IMGUI_PATH=external\imgui
set IMGPLOT_PATH=external\implot
set OPENGL_PATH=external\opengl

rem Check for release or debug argument
if "%1"=="release" (
    set BUILD_TYPE=Release
    set COMPILER_OPTS=/O2 /MT /EHsc /W3 /std:c++20 /DNDEBUG
    set OUTPUT_DIR=Build\Release
) else (
    set BUILD_TYPE=Debug
    set COMPILER_OPTS=/EHsc /MT /Od /W3 /std:c++20 /Zi /RTC1 /RTCs /RTCu
    rem /RTC1: Enable runtime error checks (stack checks, uninitialized variables, etc.)
    rem /RTCs: Catch stack variable usage before initialization
    rem /RTCu: Catch uninitialized variables
    set OUTPUT_DIR=Build\Debug
)

rem Include directories
set INCLUDE_OPTS=/I %SDL2_PATH%\include /I %OPENGL_PATH% /I include /I %IMGUI_PATH%\include /I %IMGUI_PATH%\backends /I %IMGPLOT_PATH%\include

rem Libraries to link
set LIB_OPTS=/link /LIBPATH:%SDL2_PATH%\lib SDL2.lib opengl32.lib user32.lib gdi32.lib shell32.lib

rem Create Build folder if it doesn't exist
if not exist Build mkdir Build
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Clean object files for release builds
if "%BUILD_TYPE%"=="Release" (
    del /Q %OUTPUT_DIR%\*.obj
)

rem Compile and link main.cpp
cl /Fo:%OUTPUT_DIR%\ /Fe:%OUTPUT_DIR%\behavior.exe /Fd:%OUTPUT_DIR%\behavior.pdb ^
    src/main.cpp ^
    %COMPILER_OPTS% ^
    %INCLUDE_OPTS% ^
    %LIB_OPTS%

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

rem Copy Assets folder to output directory
xcopy /E /I /Y assets %OUTPUT_DIR%\assets

rem Copy SDL2.dll to output directory
copy /Y %SDL2_PATH%\lib\SDL2.dll %OUTPUT_DIR%\

echo Build completed successfully.
endlocal
