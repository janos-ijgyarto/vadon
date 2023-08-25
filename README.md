# VADON
Game engine environment using CMake.

## Common requirements

- CMake version 3.21 or higher

Default configurations are provided in "CMakePresets.json" (compatible GUI apps will display these as config options).

Create "CMakeUserPresets.json" to add custom configurations (e.g "no GUI").

## Command line

Navigate to source folder, use `cmake --preset=PRESET-NAME` to generate the project with the desired configuration.

## Windows configuration

- Currently only works in Visual Studio 2022

### Visual studio
Open the project folder using VS. Toolbar will display target system, configuration, and build presets (this includes any custom presets in CMakeUserPresets.json). 

Select desired configuration, then build and run.
