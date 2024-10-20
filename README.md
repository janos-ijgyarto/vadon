# VADON

Vadon is a cross-platform general-purpose game engine implemented using C++ and the CMake build system. It offers a development environment using the latest features of C++ and modern programming paradigms and architectures.

## Current features

- Build system via CMake
- Dependency management via CPM
- Object pools and manager systems with convenient access pattern
- Graphics API abstraction (currently only supports DirectX 11)
- Render frame graphs
- Task graph and job threads
- Custom Entity component system (ECS, implementation details inspired in part by [EnTT](https://github.com/skypjack/entt))
- Custom reflection and RTTI system (inspired by [Godot](https://github.com/godotengine/godot))
- Scene system (inspired by [Godot](https://github.com/godotengine/godot))
- Application API (wrapper for platform, windows, etc.)
- Editor API (provides basic editing functionality, can be customized for projects)
- Developer GUI for dev tools and debugging (currently uses [Dear ImGui](https://github.com/ocornut/imgui))
- 2D rendering via Canvas API
- File management
- Serialization using JSON
- Demo application to showcase features
- Modular structure, allowing client code to only use the engine features they require

## Planned features

- Basic 3D rendering pipeline
- Options for both 2D and 3D rendering to allow creating custom pipelines
- Basic engine update loop
- Refined API for custom update logic, task/job based update, etc.
- Binary serialization
- Physics and audio support

## Building Vadon

### Common requirements

- CMake version 3.21 or higher

Default configurations are provided in "CMakePresets.json" (compatible GUI apps will display these as config options).

Create "CMakeUserPresets.json" to add custom configurations (e.g "no GUI").

### Command line

Navigate to source folder, use `cmake --preset=PRESET-NAME` to generate the project with the desired configuration.

### Windows configuration

- Currently only works in Visual Studio 2022

#### Visual studio
Open the project folder using VS. Toolbar will display target system, configuration, and build presets (this includes any custom presets in CMakeUserPresets.json). 

Select desired configuration, then build and run.

### Build configurations

By default, the engine offers configurations for dynamic linking, which can be useful during development. For released builds, static linking is recommended.

## Project structure
### Engine
Core project, contains all the context-agnostic features
#### Common
Base library, provides common features for all other modules.
- Engine core interface
- Engine subsystem interface
- ECS API
- Scene API
- Task system
- Utilities
#### Render
Mid-level renderer API, provides features that composite low-level API features.
- Render frame graph
- Utilities
#### Graphics API
Low-level renderer API and abstraction layer.
- Dummy graphics API (for debugging or when run without graphics)
- DirectX graphics API
#### Core
Implementations for engine interfaces, creates core object for client code
### Application
For projects that want to delegate platform management (windows, inputs, etc.), this library provides an abstraction layer, along with some extra features.

Currently only supports a single window being used as the "main window".

Also provides immediate mode developer GUI.
### Editor
Derived from Application, allows clients to create and edit scenes via ECS. Custom Component types can be registered and will be available in the editor as well.
### Demo
Optional projects to showcase how Vadon can be used to create a game application, as well as the relevant dev tools.
#### Common
Shared code, including the custom Component types for the ECS and the logic of the game model.
#### Editor
This project derives from the Editor library and combines it with the Demo game model in Common. It also runs custom logic (e.g rendering) while editing based on the scene contents.
#### Game
This project derives from the Application library and combines with the Demo game model in Common to create a game instance. It loads content created in the editor and runs the game simulation.
## Importing Vadon

The included demo projects illustrate how Vadon can be used to create a game. Set up an executable target, then import the engine as a library.

The CMake script contains options that can be set for when the engine is imported as a third-party package into a project.
