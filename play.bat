@echo off
REM Quick launcher (double-click). The Vietnamese menu lives in scripts\play.ps1;
REM this file stays ASCII because cmd parses .bat files in the OEM codepage.
REM Usage: play.bat          -> menu
REM        play.bat 1..9     -> run one option directly (see scripts\play.ps1)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\play.ps1" %*
