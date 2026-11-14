@echo off
REM Build all targets (caro_gui, caro_bench, caro_tests) in Release mode.
REM Requires Visual Studio 2022/2026 with "Desktop development with C++".
setlocal
cd /d "%~dp0"

set "CMAKE_EXE=cmake"
where cmake >nul 2>nul
if errorlevel 1 (
  for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -property installationPath`) do set "VSDIR=%%i"
  call set "CMAKE_EXE=%%VSDIR%%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

"%CMAKE_EXE%" -S . -B build -A x64 || exit /b 1
"%CMAKE_EXE%" --build build --config Release --parallel || exit /b 1

echo.
echo Build OK. Executables are in build\Release\
endlocal
