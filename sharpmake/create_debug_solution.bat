@echo off
echo Creating Sharpmake debug solution for Vadon
PowerShell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0sharpmake_entrypoint.ps1" -sharpmakeCommand "generateDebugSolution"
pause