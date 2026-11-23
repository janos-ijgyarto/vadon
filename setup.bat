@echo off
echo Running setup for Vadon project
PowerShell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0sharpmake/sharpmake_entrypoint.ps1" -sharpmakeCommand "setup"
pause