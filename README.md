# VADON

Vadon is a cross-platform general-purpose game engine implemented using C++ and the Sharpmake build system. It offers a development environment using the latest features of C++ and modern programming paradigms and architectures.

## Current features

- Build system via Sharpmake and FastBuild (https://github.com/ubisoft/Sharpmake) (https://www.fastbuild.org)
- Dependency management using vcpkg (https://vcpkg.io)
- Object pools and manager systems with convenient access pattern
- Graphics API abstraction (currently only supports DirectX 11)
- Render frame graphs
- Task graph and job threads
- Custom Entity component system (ECS, implementation details inspired in part by [EnTT](https://github.com/skypjack/entt))
- Custom reflection and RTTI system (inspired by [Godot](https://github.com/godotengine/godot))
- Model system (inspired by resources and nodes in [Godot](https://github.com/godotengine/godot))
- Application API (wrapper for platform, windows, etc.)
- Separate GUI editor application using Qt (https://www.qt.io/)
- Editor API (provides basic editing functionality, can be customized for projects)
- Developer GUI for dev tools and debugging (currently uses [Dear ImGui](https://github.com/ocornut/imgui))
- 2D rendering via Canvas API
- File management
- Serialization using JSON and custom binary format
- Demo application to showcase features
- Modular structure, allowing client code to only use the engine features they require
- **100% human-written, no AI tools are used for this project**
## Planned features

- Basic 3D rendering pipeline
- Options for both 2D and 3D rendering to allow creating custom pipelines
- Basic engine update loop
- Refined API for custom update logic, task/job based update, etc.
- Physics and audio support

## Building Vadon

### Common requirements
- .NET 8 (for Sharpmake)
- Windows
  - MSVC v143
  - SDK for Windows 11 - 26100
- Sharpmake (latest version built from source)
- vcpkg (latest version built from source)
- Qt 6.9.0 (dynamic)
### Setup {#sharpmake-setup}
1. Clone the repository
2. Navigate to the `sharpmake` folder
3. Make a copy of the `sharpmake_config_template.json` file
4. Rename the copy to `sharpmake_config.json`
5. Set the paths to each dependency to match your installations
### OPTIONAL: Sharpmake developer setup
1. Open a command prompt in the `sharpmake` folder
2. Run `create_debug_solution.bat "<path/to/Sharpmake.Application.exe>"` (provide the path to your Sharpmake executable)
3. The debug solution should be generated under `generated/debugsolution`
### Generate and build
1. Open a command prompt in the project root
2. Run `setup.bat "<path/to/Sharpmake.Application.exe>"` (provide the path to your Sharpmake executable)
3. The results will be generated under the `output` folder
4. `engine` contains the engine and demo app, `tools` contains the editor and other tools used during the build
5. The projects can be edited and built using Visual Studio 2022, or edited in your IDE of choice and built directly via FastBuild
## Project structure
### Foundation
Shared APIs and definitions for the engine and tools, used as a common source-of-truth for systems that need to communicate with both.
### Engine
Core project, contains all the context-agnostic features
#### Common
Base library, provides common features for all other modules.
- Engine core interface
- Engine subsystem interface
- RTTI and reflection system
- ECS API
- Model API
- Task system
- Utilities
#### Render
Mid-level renderer API, provides features that composite low-level API features.
- Canvas
- Render frame graph
- Utilities
#### Graphics API
Low-level renderer API and abstraction layer.
- Null graphics API (for debugging or when run without graphics)
- DirectX graphics API
#### Core
Implementations for engine interfaces, creates core object for client code
### Application
For projects that want to delegate platform management (windows, inputs, etc.), this library provides an abstraction layer, along with some extra features.

Currently only supports a single window being used as the "main window".

Also provides immediate mode developer GUI.
### Editor plugin
Shared logic for creating a project-specific editor plugin. Provides foundations for exporting the project data schema, and for running either the simulator or the asset server for the editor.
### Demo
Optional projects to showcase how Vadon can be used to create a game application, as well as the relevant dev tools.
#### Common
Shared code, including the custom Component types for the ECS and the logic of the game model.
#### Editor
Implements project-specific view and rendering logic to be used by the editor. When the plugin is loaded, it exports the data schema for the demo project, renders to the viewport provided by the editor, and exports the finalized assets.
#### Game
This project derives from the Application library and combines with the Demo game model in Common to create a game instance. It loads content created in the editor and runs the game simulation.
### Tools
#### Shader compiler
Command line tool that can take shader source code and compile into bytecode. Currently only supports HLSL using D3DCompile.

The engine has built-in shaders used by various modules, which are pre-compiled by this tool and exported to generated C++ files as a pre-build step.
#### Editor
Qt application that can create and load project files, and manages the resources and assets of a project. 

The application itself is project-agnostic. It relies on project-specific plugins to provide it with necessary metadata and features.

Once a project has provided an up-to-date data schema, the editor can work "offline" without running any project-specific code. All project resources can be edited as nested tables of properties.
## Importing Vadon
### Sharpmake
1. Clone the Vadon repository
2. In your own project, create a setup script similar to the one referenced in the [setup guide](#sharpmake-setup)
3. When setting the `/sources` parameter, add `<path/to/vadon/checkout>/sharpmake/vadon.sharpmake.cs`. This is an entrypoint file that will import the rest of the Vadon Sharpmake scripts.
4. Provide a config file, or set all the required parameters manually
5. You can now link Vadon dependencies directly to your project's components
### Other build systems
Use the [setup](#sharpmake-setup) to generate and build the engine libraries, then link to them using the import mechanism of your build scripts.
## Contribution Guidelines
Fixes and suggestions are welcome. Before proceeding with a pull request, please propose as an issue on GitHub for visibility and open discussion.

**AI-generated code, documentation, or suggestions will NOT be accepted.**