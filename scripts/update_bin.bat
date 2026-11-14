@echo off
REM Build Release, run the tests, then copy the executables + assets into bin\
REM (prebuilt copy that runs without Visual Studio). ASCII only: cmd parses .bat files in the OEM codepage.
setlocal
cd /d "%~dp0.."

call "%~dp0..\build.bat"
if errorlevel 1 exit /b 1

build\Release\caro_tests.exe
if errorlevel 1 (
  echo Tests FAILED - bin\ not updated
  exit /b 1
)

if not exist bin mkdir bin
copy /y build\Release\caro_gui.exe bin\ >nul || exit /b 1
copy /y build\Release\caro_bench.exe bin\ >nul || exit /b 1
copy /y build\Release\caro_tests.exe bin\ >nul || exit /b 1
xcopy /e /i /y /q assets bin\assets >nul || exit /b 1
echo bin\ updated.
endlocal
