@echo off
echo Running Sharpmake for Vadon
set USER_CONFIG_PATH=%~dp0sharpmake_config.json
pushd %~dp0
set SHARPMAKE_EXE=%~1
%SHARPMAKE_EXE% "/sources(@'vadon.sharpmake.cs') /userConfigPath(@'%USER_CONFIG_PATH%') /logAllGeneratedSolutions"
popd