@echo off
pushd %~dp0
echo Running setup for Vadon project
call sharpmake/run_sharpmake.bat %*
popd
pause