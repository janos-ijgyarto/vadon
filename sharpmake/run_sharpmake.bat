@echo off
echo Running Sharpmake for Vadon
set USER_CONFIG_PATH=%~dp0sharpmake_config.json
pushd %~dp0
set SHARPMAKE_EXE=%~1
%SHARPMAKE_EXE% "/sources(@'vadon_main.sharpmake.cs') /vadonUserConfigPath(@'%USER_CONFIG_PATH%') /logAllGeneratedSolutions"
popd