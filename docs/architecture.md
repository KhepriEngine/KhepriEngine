# Architecture

Khepri is collection of interacting components that can be instantiated by an application.
A default collection of components is provided that perform the most-often used functionality.
There are no hard-coded dependencies between components that contain state, allowing anyone using
Khepri to choose to use the defaults, or override them for custom functionality.

## Utility components
Utility components provide easy-to-use classes to simplify common tasks. These can be used by the
top-level application or other components. Unlike core components, these components cannot be swapped
for other implementations, but some do support overriding certain behaviors.

* Application Support: classes for setting up a top-level application (e.g. error handling & reporting)
* Execution: classes that enable components to control how their code is executed.
* Logging: classes that allow components to log events and debugging information.
* Math: a collection of mathematics-related classes.
* Profiling: classes that allow other components and the application to profile the engine.
* Memory: classes that provide a variety of memory allocators.

## Core components
Core components interface with the system and provide the basic functionality upon which the rest of the system is built.

* Audio: classes for interacting with the audio system (e.. playing music, sound effects)
* Configuration: classes for reading application configuration, including real-time update support.
* Networking: classes to handle networking code. Matchmaking, game synchronization, etc.
* Renderer: classes that deal with the video renderer.
* Virtual File System: classes that create a single logical file system from a collection of data sources.
* User Interface: classes that allow the creation of simple or complex UIs.
* Scripting: classes that interact with the scripting system.

## High-level components
High-level components provide more advanced logic that do not directly interact with the system.

* AI: classes for creating AIs.
* Game State: classes for representing and interacting with the game state.
