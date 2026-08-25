@echo off
echo Creating Sharpmake debug solution for Vadon
set SHARPMAKE_EXE=%~1
set USER_CONFIG_PATH=%~dp0sharpmake_config.json
pushd %~dp0
%SHARPMAKE_EXE% "/sources(@'vadon.sharpmake.cs') /verbose /generateDebugSolution /debugSolutionPath(@'generated/debugsolution') /debugSolutionDevEnv('vs2022') /generateDebugSolutionOnly"
popd
pause