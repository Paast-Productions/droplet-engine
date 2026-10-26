# Index {#mainpage}

**Droplet** is the name of the game engine built for *Project Thalassa*.

This doxygen-generated site combines our documentation of the C++ API with project-level notes and examples. Use the search box and the navigation tree to find types, namespaces, files, and members.

## Projects

Droplet contains two "main" projects, **Droplet** and **Game**.

### Droplet

Core game engine. Contains most of the functionality one would expect from such a game engine: Window management; Rendering; User input; Resource handling; Script management; Editor functionality and more.

#### Subsystems

| Subsystem | Purpose |
| ------ | ---------- |
| **Core** | Provide functionality or types that are shared through the entire Engine without being dependent on any engine subsystems. |
| **Debug** | Debugging functionality such as logging, performance measuring / profiling, drawing debug shapes. |
| **Editor** | Functionality required for the Editor used to build Scenes for a game. |
| **Engine** | Engine Instance definition, providing a single interface for engine functionality. |
| **Graphics** | SDL Window and Vulkan rendering abstraction and implementation. |
| **Math** | Maffs. Types and functions. Falls under Core. |
| **Resource** | Resource Manager and loaders for different type of resource files. |
| **SceneSystem** | Scene management and construction of scenes with node-tree hierarchies, nodes + components, and behaviours. |
| **ScriptSystem** | Lua script management, loading and bindings. |

### Game

'Custom' implementation containing scripts, behaviours and components specific to the *Project Thalassa* game. Creates Scenes and glues them together, then uses Engine instance at run time.

## Markdown Documentation

\subpage documentation_guides "Browse all documentation guides"
